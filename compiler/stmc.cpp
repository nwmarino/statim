#include "include/logger.hpp"
#include "input_file.hpp"
#include "options.hpp"
#include "parser.hpp"
#include "translation_unit.hpp"
#include "visitor.hpp"

using namespace stm;

i32 main(i32 argc, char **argv) {
    Logger::init();

    Options options;
    options.arch = Arch::X86_64;
    options.backend = Backend::LLVM;
    options.os = OpSys::Linux;
    options.pOutput = "main";
    options.debug = 1;
    options.devel = 1;
    options.emit_asm = 1;
    options.keep_obj = 1;
    options.time = 1;

    InputFile file;
    file.pPath = "samples/return_zero.stm";

    TranslationUnit unit { file };

    Parser parser { file };
    parser.parse(unit);

    Root& root = unit.get_root();
    root.validate();

    SymbolAnalysis syma { options, root };
    root.accept(syma);

    SemanticAnalysis sema { options, root };
    root.accept(sema);

    root.print(std::cout);

    Codegen codegen { options, root };
    codegen.run(unit);

    unit.get_frame().print(std::cout);

    return 0;
}
