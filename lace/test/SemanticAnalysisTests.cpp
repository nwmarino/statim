//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/lexer/Lexer.h"
#include "lace/parser/Parser.h"
#include "lace/tree/AST.h"
#include "lace/tree/Defn.h"
#include "lace/tree/SemanticAnalysis.h"
#include "lace/tree/SymbolAnalysis.h"

#include "gtest/gtest.h"

namespace lace::test {

class SemanticAnalysisTests : public ::testing::Test {
protected:
    Options opts;
    AST* ast;

    void SetUp() override {
        opts = Options();
        ast = nullptr;
    }

    void TearDown() override {
        if (ast) { 
            delete ast;
            ast = nullptr;
        }
    }
};

TEST_F(SemanticAnalysisTests, MainCheck_ReturnType_Positive) {
    TokenStream stream;
    Lexer lexer("main :: () -> s64;");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    SymbolAnalysis syma(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(syma));

    SemanticAnalysis sema(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(sema));
}

TEST_F(SemanticAnalysisTests, MainCheck_ReturnType_Negative) {
    TokenStream stream;
    Lexer lexer("main :: () -> s8;");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    SymbolAnalysis syma(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(syma));

    SemanticAnalysis sema(opts);
    EXPECT_DEATH(ast->accept(sema), "");
}

TEST_F(SemanticAnalysisTests, ConditionCheck_IfCondition_Positive) {
    TokenStream stream;
    Lexer lexer("foo :: () -> s64 { if 1 { ret 0; } }");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    SymbolAnalysis syma(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(syma));

    SemanticAnalysis sema(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(sema));
}

TEST_F(SemanticAnalysisTests, ConditionCheck_IfCondition_Negative) {
    TokenStream stream;
    Lexer lexer("bar :: () -> void; foo :: () -> s64 { if bar() { ret 0; } }");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    SymbolAnalysis syma(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(syma));

    SemanticAnalysis sema(opts);
    EXPECT_DEATH(ast->accept(sema), "");
}

TEST_F(SemanticAnalysisTests, ConditionCheck_UntilCondition_Positive) {
    TokenStream stream;
    Lexer lexer("foo :: () -> s64 { until 1 restart; }");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    SymbolAnalysis syma(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(syma));

    SemanticAnalysis sema(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(sema));
}

TEST_F(SemanticAnalysisTests, ConditionCheck_UntilCondition_Negative) {
    TokenStream stream;
    Lexer lexer("bar :: () -> void; foo :: () -> s64 { until bar() restart; }");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    SymbolAnalysis syma(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(syma));

    SemanticAnalysis sema(opts);
    EXPECT_DEATH(ast->accept(sema), "");
}

TEST_F(SemanticAnalysisTests, ControlCheck_StopInLoop_Positive) {
    TokenStream stream;
    Lexer lexer("foo :: () -> s64 { until 1 stop; }");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    SymbolAnalysis syma(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(syma));

    SemanticAnalysis sema(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(sema));
}

TEST_F(SemanticAnalysisTests, ControlCheck_StopInLoop_Negative) {
    TokenStream stream;
    Lexer lexer("foo :: () -> s64 { stop; }");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    SymbolAnalysis syma(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(syma));

    SemanticAnalysis sema(opts);
    EXPECT_DEATH(ast->accept(sema), "");
}

TEST_F(SemanticAnalysisTests, ControlCheck_RestartInLoop_Positive) {
    TokenStream stream;
    Lexer lexer("foo :: () -> s64 { until 1 restart; }");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    SymbolAnalysis syma(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(syma));

    SemanticAnalysis sema(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(sema));
}

TEST_F(SemanticAnalysisTests, ControlCheck_RestartInLoop_Negative) {
    TokenStream stream;
    Lexer lexer("foo :: () -> s64 { restart; }");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    SymbolAnalysis syma(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(syma));

    SemanticAnalysis sema(opts);
    EXPECT_DEATH(ast->accept(sema), "");
}

TEST_F(SemanticAnalysisTests, TypeCheck_VariableInitializer_Positive) {
    TokenStream stream;
    Lexer lexer("test :: () -> void { let x: s64 = 1; }");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    SymbolAnalysis syma(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(syma));

    SemanticAnalysis sema(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(sema));
}

TEST_F(SemanticAnalysisTests, TypeCheck_VariableInitializer_Negative) {
    TokenStream stream;
    Lexer lexer("test :: () -> void { let x: s64 = \"test\"; }");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    SymbolAnalysis syma(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(syma));

    SemanticAnalysis sema(opts);
    EXPECT_DEATH(ast->accept(sema), "");
}

TEST_F(SemanticAnalysisTests, TypeCheck_VoidReturn_Positive) {
    TokenStream stream;
    Lexer lexer("foo :: () -> void { ret; }");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    SymbolAnalysis syma(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(syma));

    SemanticAnalysis sema(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(sema));
}

TEST_F(SemanticAnalysisTests, TypeCheck_VoidReturn_Negative) {
    TokenStream stream;
    Lexer lexer("foo :: () -> s64 { ret; }");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    SymbolAnalysis syma(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(syma));

    SemanticAnalysis sema(opts);
    EXPECT_DEATH(ast->accept(sema), "");
}

TEST_F(SemanticAnalysisTests, TypeCheck_Return_Positive) {
    TokenStream stream;
    Lexer lexer("foo :: () -> s64 { ret 1; }");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    SymbolAnalysis syma(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(syma));

    SemanticAnalysis sema(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(sema));
}

TEST_F(SemanticAnalysisTests, TypeCheck_Return_Negative) {
    TokenStream stream;
    Lexer lexer("foo :: () -> s64 { ret \"test\"; }");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    SymbolAnalysis syma(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(syma));

    SemanticAnalysis sema(opts);
    EXPECT_DEATH(ast->accept(sema), "");
}

TEST_F(SemanticAnalysisTests, TypeCheck_CastReturn_Positive) {
    TokenStream stream;
    Lexer lexer("foo :: () -> f32 { ret 1; }");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    SymbolAnalysis syma(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(syma));

    SemanticAnalysis sema(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(sema));
}

} // namespace lace::test
