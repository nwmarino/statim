//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/lexer/Lexer.h"
#include "lace/parser/Parser.h"
#include "lace/tree/AST.h"
#include "lace/tree/SymbolAnalysis.h"

#include "gtest/gtest.h"

namespace lace::test {

class SymbolAnalysisTests : public ::testing::Test {
protected:
    Options opts;
    AST* ast;

    void SetUp() override {
        opts = {};
        ast = nullptr;
    }

    void TearDown() override {
        if (ast)
            delete ast;
            
        ast = nullptr;
    }
};

TEST_F(SymbolAnalysisTests, VariableRef_Positive) {
    TokenStream stream;
    Lexer lexer("test :: () -> s64 { let x: s64 = 0; ret x; }");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    ASSERT_NO_FATAL_FAILURE(ast = parser.parse());

    SymbolAnalysis syma(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(syma));
}

TEST_F(SymbolAnalysisTests, VariableRef_Negative) {
    TokenStream stream;
    Lexer lexer("test :: () -> s64 { let x: s64 = 0; ret y; }");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    ASSERT_NO_FATAL_FAILURE(ast = parser.parse());

    SymbolAnalysis syma(opts);
    EXPECT_DEATH(ast->accept(syma), "");
}

TEST_F(SymbolAnalysisTests, CalleeRef_Positive) {
    TokenStream stream;
    Lexer lexer("foo :: () -> s64 { ret bar(); } bar :: () -> s64 { ret 0; }");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    ASSERT_NO_FATAL_FAILURE(ast = parser.parse());

    SymbolAnalysis syma(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(syma));
}

TEST_F(SymbolAnalysisTests, ParamRef_Positive) {
    TokenStream stream;
    Lexer lexer("foo :: (a: s64) -> s64 { ret a; }");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    ASSERT_NO_FATAL_FAILURE(ast = parser.parse());

    SymbolAnalysis syma(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(syma));
}

} // namespace lace::test
