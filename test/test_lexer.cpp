#include "../compiler/include/lexer.hpp"

#include <gtest/gtest.h>

namespace stm {

namespace test {

class LexerTest : public ::testing::Test {};

TEST_F(LexerTest, lex_identifier) {
    InputFile file;
    file.pPath = "README.md";
    Lexer lexer { file, "test" };
    
    lexer.lex();
    EXPECT_EQ(lexer.last().kind, TOKEN_KIND_IDENTIFIER);
    EXPECT_EQ(lexer.last().value, "test");
}

} // namespace test

} // namespace stm
