//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lace/lexer/Lexer.h"
#include "lace/lexer/Token.h"

#include "gtest/gtest.h"

namespace lace::test {

class LexerTests : public ::testing::Test {};

TEST_F(LexerTests, Identifier) {
    Lexer lexer("test");
    Token token;

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::Identifier);
    EXPECT_EQ(token.value, "test");

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::EndOfFile);
}

TEST_F(LexerTests, MultipleIdentifiers) {
    Lexer lexer("one_ _two three_ _four");
    Token token;

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::Identifier);
    EXPECT_EQ(token.value, "one_");

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::Identifier);
    EXPECT_EQ(token.value, "_two");

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::Identifier);
    EXPECT_EQ(token.value, "three_");

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::Identifier);
    EXPECT_EQ(token.value, "_four");
}

TEST_F(LexerTests, IntegerLiteral) {
    Lexer lexer("1 0u 1L 5ul");
    Token token;

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::Integer);
    EXPECT_EQ(token.value, "1");

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::Integer);
    EXPECT_EQ(token.value, "0");

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::Identifier);
    EXPECT_EQ(token.value, "u");

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::Integer);
    EXPECT_EQ(token.value, "1");

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::Identifier);
    EXPECT_EQ(token.value, "L");

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::Integer);
    EXPECT_EQ(token.value, "5");

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::Identifier);
    EXPECT_EQ(token.value, "ul");
}

TEST_F(LexerTests, FloatLiteral) {
    Lexer lexer("1.0 1.f .1 3.14F");
    Token token;

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::Float);
    EXPECT_EQ(token.value, "1.0");

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::Float);
    EXPECT_EQ(token.value, "1.");

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::Identifier);
    EXPECT_EQ(token.value, "f");

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::Float);
    EXPECT_EQ(token.value, ".1");

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::Float);
    EXPECT_EQ(token.value, "3.14");

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::Identifier);
    EXPECT_EQ(token.value, "F");
}

TEST_F(LexerTests, CharacterLiteral) {
    Lexer lexer("'a' '0'");
    Token token;

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::Character);
    EXPECT_EQ(token.value, "a");

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::Character);
    EXPECT_EQ(token.value, "0");
}

TEST_F(LexerTests, CharacterEscapeSequences) {
    Lexer lexer("'\v' '\n' '\t' '\''");
    Token token;

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::Character);
    EXPECT_EQ(token.value, "\v");

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::Character);
    EXPECT_EQ(token.value, "\n");

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::Character);
    EXPECT_EQ(token.value, "\t");

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::Character);
    EXPECT_EQ(token.value, "'");
}

TEST_F(LexerTests, StringLiteral) {
    Lexer lexer("\"hello, world!\"");
    Token token;

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::String);
    EXPECT_EQ(token.value, "hello, world!");
}

TEST_F(LexerTests, StringEscapeSequences) {
    Lexer lexer("\"hello,\tworld!\n\"");
    Token token;

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::String);
    EXPECT_EQ(token.value, "hello,\tworld!\n");
}

TEST_F(LexerTests, IsolatedToken) {
    Lexer lexer(".");
    Token token;

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::Dot);
}

TEST_F(LexerTests, CompoundToken) {
    Lexer lexer("->");
    Token token;

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::Arrow);
}

TEST_F(LexerTests, Complete) {
    Lexer lexer("main :: (argc: s32, argv: **char) { ret argc * 3; }");
    Token token;

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::Identifier);
    EXPECT_EQ(token.value, "main");

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::Path);

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::OpenParen);

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::Identifier);
    EXPECT_EQ(token.value, "argc");

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::Colon);

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::Identifier);
    EXPECT_EQ(token.value, "s32");

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::Comma);

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::Identifier);
    EXPECT_EQ(token.value, "argv");

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::Colon);

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::Star);

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::Star);

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::Identifier);
    EXPECT_EQ(token.value, "char");

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::CloseParen);

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::OpenBrace);

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::Identifier);
    EXPECT_EQ(token.value, "ret");

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::Identifier);
    EXPECT_EQ(token.value, "argc");

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::Star);

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::Integer);
    EXPECT_EQ(token.value, "3");

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::Semi);

    lexer.lex(token);
    EXPECT_EQ(token.kind, Token::CloseBrace);
}

} // namespace lace::test
