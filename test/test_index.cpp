#include "../compiler/include/types.hpp"

#include <gtest/gtest.h>

stm::i32 main(stm::i32 argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
