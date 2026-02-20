//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/lexer/Lexer.h"
#include "lace/lexer/Token.h"

#include "gtest/gtest.h"

namespace lace::test {

class LexerTests : public ::testing::Test {};

TEST_F(LexerTests, Identifier) {
    TokenStream stream;
    Lexer lexer("test");
    ASSERT_TRUE(lexer.lex(stream));

    const Token& T1 = stream.get();
    EXPECT_EQ(T1.kind, Token::Identifier);
    EXPECT_EQ(T1.value, "test");
    stream.advance();

    EXPECT_TRUE(stream.complete());
}

TEST_F(LexerTests, MultipleIdentifiers) {
    TokenStream stream;
    Lexer lexer("one_ _two three_ _four");
    ASSERT_TRUE(lexer.lex(stream));

    const Token& T1 = stream.get();
    EXPECT_EQ(T1.kind, Token::Identifier);
    EXPECT_EQ(T1.value, "one_");
    stream.advance();

    const Token& T2 = stream.get();
    EXPECT_EQ(T2.kind, Token::Identifier);
    EXPECT_EQ(T2.value, "_two");
    stream.advance();

    const Token& T3 = stream.get();
    EXPECT_EQ(T3.kind, Token::Identifier);
    EXPECT_EQ(T3.value, "three_");
    stream.advance();

    const Token& T4 = stream.get();
    EXPECT_EQ(T4.kind, Token::Identifier);
    EXPECT_EQ(T4.value, "_four");
    stream.advance();

    EXPECT_TRUE(stream.complete());
}

TEST_F(LexerTests, IntegerLiteral) {
    TokenStream stream;
    Lexer lexer("1 0u 1L 5ul");
    ASSERT_TRUE(lexer.lex(stream));

    const Token& T1 = stream.get();
    EXPECT_EQ(T1.kind, Token::Integer);
    EXPECT_EQ(T1.value, "1");
    stream.advance();

    const Token& T2 = stream.get();
    EXPECT_EQ(T2.kind, Token::Integer);
    EXPECT_EQ(T2.value, "0");
    stream.advance();

    const Token& T3 = stream.get();
    EXPECT_EQ(T3.kind, Token::Identifier);
    EXPECT_EQ(T3.value, "u");
    stream.advance();

    const Token& T4 = stream.get();
    EXPECT_EQ(T4.kind, Token::Integer);
    EXPECT_EQ(T4.value, "1");
    stream.advance();

    const Token& T5 = stream.get();
    EXPECT_EQ(T5.kind, Token::Identifier);
    EXPECT_EQ(T5.value, "L");
    stream.advance();

    const Token& T6 = stream.get();
    EXPECT_EQ(T6.kind, Token::Integer);
    EXPECT_EQ(T6.value, "5");
    stream.advance();

    const Token& T7 = stream.get();
    EXPECT_EQ(T7.kind, Token::Identifier);
    EXPECT_EQ(T7.value, "ul");
    stream.advance();

    EXPECT_TRUE(stream.complete());
}

TEST_F(LexerTests, FloatLiteral) {
    TokenStream stream;
    Lexer lexer("1.0 1.f .1 3.14F");
    ASSERT_TRUE(lexer.lex(stream));

    const Token& T1 = stream.get();
    EXPECT_EQ(T1.kind, Token::Float);
    EXPECT_EQ(T1.value, "1.0");
    stream.advance();

    const Token& T2 = stream.get();
    EXPECT_EQ(T2.kind, Token::Float);
    EXPECT_EQ(T2.value, "1.");
    stream.advance();

    const Token& T3 = stream.get();
    EXPECT_EQ(T3.kind, Token::Identifier);
    EXPECT_EQ(T3.value, "f");
    stream.advance();

    const Token& T4 = stream.get();
    EXPECT_EQ(T4.kind, Token::Float);
    EXPECT_EQ(T4.value, ".1");
    stream.advance();

    const Token& T5 = stream.get();
    EXPECT_EQ(T5.kind, Token::Float);
    EXPECT_EQ(T5.value, "3.14");
    stream.advance();

    const Token& T6 = stream.get();
    EXPECT_EQ(T6.kind, Token::Identifier);
    EXPECT_EQ(T6.value, "F");
    stream.advance();

    EXPECT_TRUE(stream.complete());
}

TEST_F(LexerTests, CharacterLiteral) {
    TokenStream stream;
    Lexer lexer("'a' '0'");
    ASSERT_TRUE(lexer.lex(stream));

    const Token& T1 = stream.get();
    EXPECT_EQ(T1.kind, Token::Character);
    EXPECT_EQ(T1.value, "a");
    stream.advance();

    const Token& T2 = stream.get();
    EXPECT_EQ(T2.kind, Token::Character);
    EXPECT_EQ(T2.value, "0");
    stream.advance();
    
    EXPECT_TRUE(stream.complete());
}

TEST_F(LexerTests, CharacterEscapeSequences) {
    TokenStream stream;
    Lexer lexer("'\v' '\n' '\t' '\''");
    ASSERT_TRUE(lexer.lex(stream));

    const Token& T1 = stream.get();
    EXPECT_EQ(T1.kind, Token::Character);
    EXPECT_EQ(T1.value, "\v");
    stream.advance();

    const Token& T2 = stream.get();
    EXPECT_EQ(T2.kind, Token::Character);
    EXPECT_EQ(T2.value, "\n");
    stream.advance();

    const Token& T3 = stream.get();
    EXPECT_EQ(T3.kind, Token::Character);
    EXPECT_EQ(T3.value, "\t");
    stream.advance();

    const Token& T4 = stream.get();
    EXPECT_EQ(T4.kind, Token::Character);
    EXPECT_EQ(T4.value, "'");
    stream.advance();

    EXPECT_TRUE(stream.complete());
}

TEST_F(LexerTests, StringLiteral) {
    TokenStream stream;
    Lexer lexer("\"hello, world!\"");
    ASSERT_TRUE(lexer.lex(stream));

    const Token& T1 = stream.get();
    EXPECT_EQ(T1.kind, Token::String);
    EXPECT_EQ(T1.value, "hello, world!");
    stream.advance();

    EXPECT_TRUE(stream.complete());
}

TEST_F(LexerTests, StringEscapeSequences) {
    TokenStream stream;
    Lexer lexer("\"hello,\tworld!\n\"");
    ASSERT_TRUE(lexer.lex(stream));

    const Token& T1 = stream.get();
    EXPECT_EQ(T1.kind, Token::String);
    EXPECT_EQ(T1.value, "hello,\tworld!\n");
    stream.advance();

    EXPECT_TRUE(stream.complete());
}

TEST_F(LexerTests, IsolatedToken) {
    TokenStream stream;
    Lexer lexer(".");
    ASSERT_TRUE(lexer.lex(stream));

    const Token& T1 = stream.get();
    EXPECT_EQ(T1.kind, Token::Dot);
    stream.advance();

    EXPECT_TRUE(stream.complete());
}

TEST_F(LexerTests, CompoundToken) {
    TokenStream stream;
    Lexer lexer("->");
    ASSERT_TRUE(lexer.lex(stream));

    const Token& T1 = stream.get();
    EXPECT_EQ(T1.kind, Token::Arrow);
    stream.advance();

    EXPECT_TRUE(stream.complete());
}

TEST_F(LexerTests, Complete1) {
    TokenStream stream;
    Lexer lexer("main :: (argc: s32, argv: **char) { ret argc * 3; }");
    ASSERT_TRUE(lexer.lex(stream));

    const Token& T1 = stream.get();
    EXPECT_EQ(T1.kind, Token::Identifier);
    EXPECT_EQ(T1.value, "main");
    stream.advance();

    const Token& T2 = stream.get();
    EXPECT_EQ(T2.kind, Token::Path);
    stream.advance();

    const Token& T3 = stream.get();
    EXPECT_EQ(T3.kind, Token::OpenParen);
    stream.advance();

    const Token& T4 = stream.get();
    EXPECT_EQ(T4.kind, Token::Identifier);
    EXPECT_EQ(T4.value, "argc");
    stream.advance();

    const Token& T5 = stream.get();
    EXPECT_EQ(T5.kind, Token::Colon);
    stream.advance();

    const Token& T6 = stream.get();
    EXPECT_EQ(T6.kind, Token::Identifier);
    EXPECT_EQ(T6.value, "s32");
    stream.advance();

    const Token& T7 = stream.get();
    EXPECT_EQ(T7.kind, Token::Comma);
    stream.advance();

    const Token& T8 = stream.get();
    EXPECT_EQ(T8.kind, Token::Identifier);
    EXPECT_EQ(T8.value, "argv");
    stream.advance();

    const Token& T9 = stream.get();
    EXPECT_EQ(T9.kind, Token::Colon);
    stream.advance();

    const Token& T10 = stream.get();
    EXPECT_EQ(T10.kind, Token::Star);
    stream.advance();

    const Token& T11 = stream.get();
    EXPECT_EQ(T11.kind, Token::Star);
    stream.advance();

    const Token& T12 = stream.get();
    EXPECT_EQ(T12.kind, Token::Identifier);
    EXPECT_EQ(T12.value, "char");
    stream.advance();

    const Token& T13 = stream.get();
    EXPECT_EQ(T13.kind, Token::CloseParen);
    stream.advance();

    const Token& T14 = stream.get();
    EXPECT_EQ(T14.kind, Token::OpenBrace);
    stream.advance();

    const Token& T15 = stream.get();
    EXPECT_EQ(T15.kind, Token::Identifier);
    EXPECT_EQ(T15.value, "ret");
    stream.advance();

    const Token& T16 = stream.get();
    EXPECT_EQ(T16.kind, Token::Identifier);
    EXPECT_EQ(T16.value, "argc");
    stream.advance();

    const Token& T17 = stream.get();
    EXPECT_EQ(T17.kind, Token::Star);
    stream.advance();

    const Token& T18 = stream.get();
    EXPECT_EQ(T18.kind, Token::Integer);
    EXPECT_EQ(T18.value, "3");
    stream.advance();

    const Token& T19 = stream.get();
    EXPECT_EQ(T19.kind, Token::Semi);
    stream.advance();

    const Token& T20 = stream.get();
    EXPECT_EQ(T20.kind, Token::CloseBrace);
    stream.advance();

    EXPECT_TRUE(stream.complete());
}

} // namespace lace::test
