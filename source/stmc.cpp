#include "core/logger.hpp"
#include "siir/cfg.hpp"
#include "siir/target.hpp"
#include "tree/parser.hpp"
#include "tree/visitor.hpp"
#include "types/input_file.hpp"
#include "types/options.hpp"
#include "types/translation_unit.hpp"

stm::i32 main(stm::i32 argc, char** argv) {
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

    root.print(std::cout);

    stm::siir::Target target { 
        stm::siir::Target::amd64, 
        stm::siir::Target::SystemV, 
        stm::siir::Target::Linux 
    };

    stm::siir::CFG cfg { file, target };

    stm::Codegen cgn { options, root, cfg };
    root.accept(cgn);

    cfg.print(std::cout);
    return 0;
}
