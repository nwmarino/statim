#include "include/logger.hpp"
#include "input_file.hpp"
#include "parser.hpp"
#include "translation_unit.hpp"
#include "visitor.hpp"

using namespace stm;

i32 main(i32 argc, char **argv) {
    Logger::init();

    Options options;

    InputFile file;
    file.pPath = "samples/return_zero.stm";

    TranslationUnit unit { file };

    Parser parser { file };

    unit.set_root(parser.get_root());

    Root& root = unit.get_root();
    root.validate();

    SymbolAnalysis syma { options, root };
    root.accept(syma);

    SemanticAnalysis sema { options, root };
    root.accept(sema);

    root.print(std::cout);

    return 0;
}
