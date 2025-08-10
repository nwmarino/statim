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
    Lexer lexer { file, "one_ _two" };

    lexer.lex();
    EXPECT_EQ(lexer.last().kind, TOKEN_KIND_IDENTIFIER);
    EXPECT_EQ(lexer.last().value, "one_");

    lexer.lex();
    EXPECT_EQ(lexer.last().kind, TOKEN_KIND_IDENTIFIER);
    EXPECT_EQ(lexer.last().value, "_two");

    lexer.lex();
    EXPECT_EQ(lexer.last().kind, TOKEN_KIND_END_OF_FILE);
}

TEST_F(LexerTest, lex_literal_character) {
    InputFile file;
    file.pPath = "test";
    Lexer lexer { file, "'a'" };

    lexer.lex();
    EXPECT_EQ(lexer.last().kind, TOKEN_KIND_CHARACTER);
    EXPECT_EQ(lexer.last().value, "a");
}

TEST_F(LexerTest, lex_literal_character_escape_sequence) {
    InputFile file;
    file.pPath = "test";
    Lexer lexer { file, "'\t'" };

    lexer.lex();
    EXPECT_EQ(lexer.last().kind, TOKEN_KIND_CHARACTER);
    EXPECT_EQ(lexer.last().value, "\t");
}

TEST_F(LexerTest, lex_literal_string) {
    InputFile file;
    file.pPath = "test";
    Lexer lexer { file, "\"hey!\"" };

    lexer.lex();
    EXPECT_EQ(lexer.last().kind, TOKEN_KIND_STRING);
    EXPECT_EQ(lexer.last().value, "hey!");
}

TEST_F(LexerTest, lex_literal_string_escape_sequence) {
    InputFile file;
    file.pPath = "test";
    Lexer lexer { file, "\"hey\nbye\t\"" };

    lexer.lex();
    EXPECT_EQ(lexer.last().kind, TOKEN_KIND_STRING);
    EXPECT_EQ(lexer.last().value, "hey\nbye\t");
}

TEST_F(LexerTest, lex_literal_integer) {
    InputFile file;
    file.pPath = "test";
    Lexer lexer { file, "1" };

    lexer.lex();
    EXPECT_EQ(lexer.last().kind, TOKEN_KIND_INTEGER);
    EXPECT_EQ(lexer.last().value, "1");
}

TEST_F(LexerTest, lex_literal_float) {
    InputFile file;
    file.pPath = "test";
    Lexer lexer { file, "3.14" };

    lexer.lex();
    EXPECT_EQ(lexer.last().kind, TOKEN_KIND_FLOAT);
    EXPECT_EQ(lexer.last().value, "3.14");
}

TEST_F(LexerTest, lex_basic_token) {
    InputFile file;
    file.pPath = "test";
    Lexer lexer { file, "." };

    lexer.lex();
    EXPECT_EQ(lexer.last().kind, TOKEN_KIND_DOT);
}

TEST_F(LexerTest, lex_compound_token) {
    InputFile file;
    file.pPath = "test";
    Lexer lexer { file, "->" };

    lexer.lex();
    EXPECT_EQ(lexer.last().kind, TOKEN_KIND_ARROW);
}

} // namespace test

} // namespace stm
