#include "core/logger.hpp"
#include "siir/cfg.hpp"
#include "siir/ssa_rewrite_pass.hpp"
#include "siir/target.hpp"
#include "siir/trivial_dce_pass.hpp"
#include "tree/parser.hpp"
#include "tree/visitor.hpp"
#include "types/input_file.hpp"
#include "types/options.hpp"
#include "types/translation_unit.hpp"
#include <fstream>

stm::i32 main(stm::i32 argc, char** argv) {
    std::ofstream pre_ssa_dump("pre_ssa");
    std::ofstream post_ssa_dump("post_ssa");
    stm::Logger::init();

    stm::Options options;
    options.output = "main";
    options.debug = 1;
    options.devel = 1;
    options.emit_asm = 1;
    options.keep_obj = 1;
    options.time = 1;

    stm::InputFile file;
    file.path = "samples/b.stm";

    stm::TranslationUnit unit { file };

    stm::Parser parser { file };
    parser.parse(unit);

    stm::Root& root = unit.get_root();
    root.validate();

    stm::SymbolAnalysis syma { options, root };
    root.accept(syma);

    stm::SemanticAnalysis sema { options, root };
    root.accept(sema);

    stm::siir::Target target { 
        stm::siir::Target::amd64, 
        stm::siir::Target::SystemV, 
        stm::siir::Target::Linux 
    };

    stm::siir::CFG cfg { file, target };

    stm::Codegen cgn { options, root, cfg };
    root.accept(cgn);

    cfg.print(pre_ssa_dump);
    pre_ssa_dump.close();

    stm::siir::SSARewrite ssar { cfg };
    ssar.run();

    stm::siir::TrivialDCEPass dce { cfg };
    //dce.run();

    cfg.print(post_ssa_dump);
    post_ssa_dump.close();
    return 0;
}
