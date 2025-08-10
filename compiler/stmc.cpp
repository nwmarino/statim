#include "include/logger.hpp"

using namespace stm;

i32 main(i32 argc, char **argv) {
    logger_init();
    logger_info("this is some info!");
    logger_warn("this is a warning!");
    logger_error("this is an error!");
    logger_fatal("this is fatal!");
    
    return 0;
}
