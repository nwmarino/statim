#include "core/logger.hpp"
#include "siir/cfg.hpp"
#include "siir/llvm_translate_pass.hpp"
#include "siir/ssa_rewrite_pass.hpp"
#include "siir/target.hpp"
#include "siir/trivial_dce_pass.hpp"
#include "tree/parser.hpp"
#include "tree/visitor.hpp"
#include "types/input_file.hpp"
#include "types/options.hpp"
#include "types/translation_unit.hpp"

#include "llvm/ADT/Twine.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/CGSCCPassManager.h"
#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/PassManager.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/CodeGen.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Triple.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"

#include <cstdlib>

void emit_module(const stm::Options& opts, llvm::CodeGenFileType file_type,
                 llvm::Module& module, llvm::TargetMachine* TM) {
    if (module.empty() && module.global_empty())
        return;

    llvm::PassBuilder PB { TM };
    llvm::LoopAnalysisManager LAM;
    llvm::FunctionAnalysisManager FAM;
    llvm::CGSCCAnalysisManager CGAM;
    llvm::ModuleAnalysisManager MAM;

    llvm::TargetLibraryInfoImpl TLII { llvm::Triple(module.getTargetTriple()) };
    TLII.disableAllFunctions();

    FAM.registerPass([&] {
        return llvm::TargetLibraryAnalysis(TLII);
    });

    FAM.registerPass([&] {
        return PB.buildDefaultAAPipeline();
    });

    PB.registerModuleAnalyses(MAM);
    PB.registerCGSCCAnalyses(CGAM);
    PB.registerFunctionAnalyses(FAM);
    PB.registerLoopAnalyses(LAM);
    PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

    llvm::ModulePassManager MPM;
    MPM.addPass(llvm::createModuleToFunctionPassAdaptor(
        llvm::SimplifyCFGPass()));

    std::string ofile;
    switch (file_type) {
    case llvm::CodeGenFileType::AssemblyFile:
        ofile = module.getSourceFileName() + ".s";
        break;
    case llvm::CodeGenFileType::ObjectFile:
        ofile = module.getSourceFileName() + ".o";
        break;
    case llvm::CodeGenFileType::Null:
        assert(false && "cannot emit null LLVM file type!");
    }

    llvm::sys::fs::OpenFlags flags = llvm::sys::fs::OF_None;
    if (file_type == llvm::CodeGenFileType::AssemblyFile)
        flags = llvm::sys::fs::OF_Text;

    std::error_code EC;
    llvm::ToolOutputFile* output = new llvm::ToolOutputFile(ofile, EC, flags);
    assert(!EC && "error occured creating LLVM output file!");

    llvm::legacy::PassManager LPM;
    assert(!TM->addPassesToEmitFile(LPM, output->os(), nullptr, file_type) &&
        "unable to add passes in order to emit LLVM module!");

    MPM.run(module, MAM);
    LPM.run(module);
    output->keep();
}

stm::i32 main(stm::i32 argc, char** argv) {
    stm::Logger::init();

    stm::Options options;
    options.output = "main";
    options.optlevel = 0;
    options.debug = true;
    options.devel = true;
    options.emit_asm = true;
    options.keep_obj = true;
    options.llvm = true;
    options.time = true;

    std::vector<std::unique_ptr<stm::InputFile>> files;
    std::vector<std::unique_ptr<stm::TranslationUnit>> units;
    
    files.push_back(std::make_unique<stm::InputFile>("samples/b.stm"));
    
    for (auto& file : files) {
        std::unique_ptr<stm::TranslationUnit> unit =
            std::make_unique<stm::TranslationUnit>(*file);
            
        stm::Parser parser { *file };
        parser.parse(*unit);

        units.push_back(std::move(unit));
    }

    /// TODO: Resolve imports at this point.

    for (auto& unit : units) {
        stm::Root& root = unit->get_root();
        root.validate();

        stm::SymbolAnalysis syma { options, root };
        root.accept(syma);

        stm::SemanticAnalysis sema { options, root };
        root.accept(sema);
    }

    stm::siir::Target target { 
        stm::siir::Target::amd64, 
        stm::siir::Target::SystemV, 
        stm::siir::Target::Linux 
    };

    for (auto& unit : units) {
        std::unique_ptr<stm::siir::CFG> graph =
            std::make_unique<stm::siir::CFG>(unit->get_file(), target);

        stm::Codegen cgn { options, unit->get_root(), *graph };
        unit->get_root().accept(cgn);

        unit->set_graph(std::move(graph));
    }

    for (auto& unit : units) {
        stm::siir::CFG& graph = unit->get_graph();

        if (options.optlevel >= 1) {
            stm::siir::SSARewrite ssar { graph };
            ssar.run();

            stm::siir::TrivialDCEPass dce { graph };
            dce.run();
        }
    }

    if (options.llvm) {
        llvm::InitializeAllTargetInfos();
        llvm::InitializeAllTargets();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmParsers();
        llvm::InitializeAllAsmPrinters();

        std::string arch_str;
        std::string vendor_str;
        std::string os_str;

        switch (target.arch()) {
        case stm::siir::Target::amd64:
            arch_str = "amd64";
            vendor_str = "AMD";
            break;
        }

        switch (target.os()) {
        case stm::siir::Target::Linux:
            os_str = "Linux";
            break;
        }

        llvm::Triple triple = llvm::Triple(arch_str, vendor_str, os_str);
        llvm::TargetOptions opts;
        std::string err;
        const llvm::Target* llvm_target = llvm::TargetRegistry::lookupTarget(
            triple.getTriple(), err);
        assert(llvm_target && "unable to find equivelant LLVM target!");

        llvm::CodeGenOptLevel opt = llvm::CodeGenOptLevel::None;
        switch (options.optlevel) {
        case 1:
            opt = llvm::CodeGenOptLevel::Less;
            break;
        case 2:
            opt = llvm::CodeGenOptLevel::Default;
            break;
        case 3:
            opt = llvm::CodeGenOptLevel::Aggressive;
            break;
        }

        llvm::TargetMachine* target_mc = llvm_target->createTargetMachine(
            triple.getTriple(), 
            "generic", 
            "", 
            opts, 
            llvm::Reloc::PIC_, 
            std::nullopt, 
            opt, 
            false);

        std::vector<std::unique_ptr<llvm::LLVMContext>> contexts;
        std::vector<std::unique_ptr<llvm::Module>> modules;
        contexts.reserve(units.size());
        modules.reserve(units.size());

        for (auto& unit : units) {
            stm::siir::CFG& graph = unit->get_graph();
            std::unique_ptr<llvm::LLVMContext> context =
                std::make_unique<llvm::LLVMContext>();
            std::unique_ptr<llvm::Module> module =
                std::make_unique<llvm::Module>(unit->get_file().absolute(), *context);

            stm::siir::LLVMTranslatePass cvt { graph, *module };
            cvt.run();

            module->setDataLayout(target_mc->createDataLayout());
            module->setTargetTriple(triple.getTriple());

            module->print(llvm::outs(), nullptr);

            emit_module(
                options, 
                llvm::CodeGenFileType::AssemblyFile, 
                *module,
                target_mc);

            emit_module(
                options, 
                llvm::CodeGenFileType::ObjectFile, 
                *module,
                target_mc);

            contexts.push_back(std::move(context));
            modules.push_back(std::move(module));
        }

        std::string ld = "ld -nostdlib -o " + std::string(options.output) + " std/rt.o ";
        for (const auto& module : modules) {
            ld += module->getSourceFileName() + ".o ";
        }

        std::system(ld.c_str());
    } else {
        assert(false && "native machine code generation not implemented!");
    }

    return 0;
}
