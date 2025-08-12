#include "include/logger.hpp"
#include "input_file.hpp"
#include "parser.hpp"

using namespace stm;

i32 main(i32 argc, char **argv) {
    logger_init();

    InputFile file;
    file.pPath = "samples/return_zero.stm";

    Parser parser { file };
    std::unique_ptr<Root> root = parser.get_root();

    root->print(std::cout);
    return 0;
}
