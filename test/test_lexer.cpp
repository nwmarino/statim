#include "../compiler/include/lexer.hpp"
#include "token.hpp"

#include <gtest/gtest.h>

namespace stm {

namespace test {

class LexerTest : public ::testing::Test {};

TEST_F(LexerTest, lex_identifier) {
    InputFile file;
    file.pPath = "test";
    Lexer lexer { file, "test" };
    
    lexer.lex();
    EXPECT_EQ(lexer.last().kind, TOKEN_KIND_IDENTIFIER);
    EXPECT_EQ(lexer.last().value, "test");

    lexer.lex();
    EXPECT_EQ(lexer.last().kind, TOKEN_KIND_END_OF_FILE);
}

TEST_F(LexerTest, lex_identifier_many) {
    InputFile file;
    file.pPath = "test";
    Lexer lexer { file, "one two" };

    lexer.lex();
    EXPECT_EQ(lexer.last().kind, TOKEN_KIND_IDENTIFIER);
    EXPECT_EQ(lexer.last().value, "one");

    lexer.lex();
    EXPECT_EQ(lexer.last().kind, TOKEN_KIND_IDENTIFIER);
    EXPECT_EQ(lexer.last().value, "two");

    lexer.lex();
    EXPECT_EQ(lexer.last().kind, TOKEN_KIND_END_OF_FILE);
}

} // namespace test

} // namespace stm
