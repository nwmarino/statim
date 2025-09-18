#include "core/logger.hpp"
#include "siir/cfg.hpp"
#include "siir/llvm_translate_pass.hpp"
#include "siir/machine_analysis.hpp"
#include "siir/machine_object.hpp"
#include "siir/ssa_rewrite_pass.hpp"
#include "siir/target.hpp"
#include "siir/trivial_dce_pass.hpp"
#include "tree/parser.hpp"
#include "tree/type.hpp"
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
#include "llvm/Transforms/Scalar/SROA.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <memory>

static stm::TranslationUnit* 
resolve_use(stm::UseDecl* use, stm::InputFile& req,
            const std::vector<std::unique_ptr<stm::TranslationUnit>>& units) {
    std::string path = use->path();

    if (path.size() < 4 || path.substr(path.size() - 4) != ".stm")
        path += ".stm";

    std::filesystem::path req_path(req.absolute());
    std::filesystem::path resolved = req_path.parent_path() / path;

    std::error_code err;
    std::filesystem::path absol = std::filesystem::canonical(resolved, err);
    if (err) 
        return nullptr;

    for (auto& unit : units) {
        if (unit->get_file().absolute() == absol.string())
            return unit.get();
    }

    return nullptr;
}

/// Takes all the public symbols which are in the file used by |use| and
/// imports them to the translation unit |dst|.
static void link_imports(stm::UseDecl* use, stm::TranslationUnit* dst) {
    assert(use && "use cannot be null!");
    assert(use->resolved() && "use must be resolved!");

    stm::Root& src_root = use->unit()->get_root();
    stm::Root& dst_root = dst->get_root();

    std::vector<stm::Decl*> src_imps = src_root.imports();
    stm::Scope* scope = dst_root.get_scope();
    stm::TypeContext& ctx = dst_root.context();

    std::vector<stm::Decl*>& dst_imps = dst_root.imports();
    for (auto& exp : use->unit()->get_root().exports()) {
        // Prevent duplicate imports.
        if (std::find(dst_imps.begin(), dst_imps.end(), exp) != dst_imps.end())
            continue;
        
        dst_imps.push_back(exp);
        
        if (!dst_root.get_scope()->add(exp)) {
            stm::Logger::fatal(
                "cannot import '" + exp->get_name() + 
                    "' since a symbol with the same name already exists",
                use->get_span());
        }

        if (use->has_decorator(stm::Rune::Public))
            dst_imps.push_back(exp);

        if (auto ST = dynamic_cast<stm::StructDecl*>(exp)) {
            
        } else if (auto ET = dynamic_cast<stm::EnumDecl*>(exp)) {
            for (auto& value : ET->get_values())
                dst_root.get_scope()->add(value);

        }
    }
}

/// Resolve the uses for the provided translation unit. Returns true if any
/// cyclical imports were found.
static bool 
resolve_uses(stm::TranslationUnit* unit, 
             std::vector<stm::TranslationUnit*> visited, 
             std::vector<stm::TranslationUnit*> stack,
             const std::vector<std::unique_ptr<stm::TranslationUnit>>& units) {

    if (std::find(visited.begin(), visited.end(), unit) != visited.end()) {
        if (!stack.empty())
            stack.pop_back();

        return false;
    }

    visited.push_back(unit);
    stack.push_back(unit);

    for (auto& use : unit->get_root().uses()) {
        stm::TranslationUnit* dep = resolve_use(
            use, unit->get_file(), units);
        
        if (!dep) {
            stm::Logger::fatal(
                "unresolved source file: '" + use->path() + "'", 
                use->get_span());
        }

        use->resolve(dep);

        if (std::find(stack.begin(), stack.end(), dep) != stack.end()) {
            stm::Logger::fatal(
                "cannot recursively use source files", use->get_span());
        }

        resolve_uses(dep, visited, stack, units);
        link_imports(use, unit);
    }

    stack.pop_back();    
    return false;
}

static void 
link_trees(const std::vector<std::unique_ptr<stm::TranslationUnit>>& units) {
    std::vector<stm::TranslationUnit*> visited = {};
    std::vector<stm::TranslationUnit*> stack = {};

    for (auto& unit : units) {
        if (std::find(visited.begin(), visited.end(), unit.get()) == visited.end())
            resolve_uses(unit.get(), visited, stack, units);
    }
}

static void emit_module(const stm::Options& opts, 
                        llvm::CodeGenFileType file_type,
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
        llvm::SROAPass(llvm::SROAOptions::ModifyCFG)));
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

    std::ofstream dump("dump");

    stm::Options options;
    options.output = "main";
    options.optlevel = 0;
    options.debug = true;
    options.devel = true;
    options.emit_asm = true;
    options.keep_obj = true;
    options.llvm = false;
    options.time = true;

    std::vector<std::unique_ptr<stm::InputFile>> files;
    std::vector<std::unique_ptr<stm::TranslationUnit>> units;

    for (stm::u32 i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-o") {
            if (++i >= argc)
                stm::Logger::fatal("expected identifier after '-o' argument to specify output name");
            
            options.output = argv[i];
        } else if (arg == "-O0") {
            options.optlevel = 0;
        } else if (arg == "-O1") {
            options.optlevel = 1;
        } else if (arg == "-O2") {
            options.optlevel = 2;
        } else if (arg == "-O3") {
            options.optlevel = 3;
        } else if (arg == "-g") {
            options.debug = true;
        } else if (arg == "-d") {
            options.devel = true;
        } else if (arg == "-S") {
            options.emit_asm = true;
        } else if (arg == "-c") {
            options.keep_obj = true;
        } else if (arg == "-ll") {
            options.llvm = true;
        } else if (arg == "-t") {
            options.time = true;
        } else {
            files.push_back(std::make_unique<stm::InputFile>(argv[i]));
        }
    }
    
    files.push_back(std::make_unique<stm::InputFile>("samples/arith.stm"));
    //files.push_back(std::make_unique<stm::InputFile>("samples/natives.stm"));
    //files.push_back(std::make_unique<stm::InputFile>("samples/b.stm"));
    //files.push_back(std::make_unique<stm::InputFile>("samples/mem.stm"));
    //files.push_back(std::make_unique<stm::InputFile>("samples/string.stm"));

    if (files.empty())
        stm::Logger::fatal("no input files");
    
    for (auto& file : files) {
        std::unique_ptr<stm::TranslationUnit> unit =
            std::make_unique<stm::TranslationUnit>(*file);
            
        stm::Parser parser { *file };
        parser.parse(*unit);

        units.push_back(std::move(unit));
    }

    link_trees(units);

    for (auto& unit : units) {
        stm::Root& root = unit->get_root();
        root.validate();
    }

    link_trees(units);

    for (auto& unit : units) {
        stm::Root& root = unit->get_root();

        stm::SymbolAnalysis syma { options, root };
        root.accept(syma);

        stm::SemanticAnalysis sema { options, root };
        root.accept(sema);

        root.print(dump);
    }

    dump.flush();

    stm::siir::Target target { 
        stm::siir::Target::x64, 
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

        graph.print(dump);

        if (!options.llvm && options.optlevel >= 1) {
            stm::siir::SSARewrite ssar { graph };
            ssar.run();

            stm::siir::TrivialDCEPass dce { graph };
            dce.run();
        }
    }

    dump.close();

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
        case stm::siir::Target::x64:
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

        std::string ld = "ld -nostdlib -o " + 
            std::string(options.output) + " std/rt.o ";
        for (const auto& module : modules)
            ld += module->getSourceFileName() + ".o ";

        std::system(ld.c_str());
    } else {
        std::vector<std::unique_ptr<stm::siir::MachineObject>> objs;

        for (auto& unit : units) {
            stm::siir::CFG& graph = unit->get_graph();
            std::unique_ptr<stm::siir::MachineObject> obj =
                std::make_unique<stm::siir::MachineObject>(&graph, &target); 
            
            stm::siir::CFGMachineAnalysis CMA { graph };
            CMA.run(*obj);

            stm::siir::FunctionRegisterAnalysis FRA { *obj };
            FRA.run();

            stm::siir::MachineObjectPrinter printer { *obj };
            printer.run(std::cout);

            std::ofstream asmf { graph.get_file().filename() + ".s" };
            assert(asmf.is_open());

            stm::siir::MachineObjectAsmWriter writer { *obj };
            writer.run(asmf);
            asmf.close();

            std::string as = "as -o " + graph.get_file().filename() + ".o " + 
                graph.get_file().filename() + ".s";
            std::system(as.c_str());
        }

        std::string ld = "ld -nostdlib -o " + 
            std::string(options.output) + " std/rt.o ";
        for (const auto& unit : units)
            ld += unit->get_graph().get_file().filename() + ".o ";

        std::system(ld.c_str());
    }

    return 0;
}
