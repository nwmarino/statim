//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

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
    Parser parser("main :: () -> s64;");
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    SymbolAnalysis syma(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(syma));

    SemanticAnalysis sema(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(sema));
}

TEST_F(SemanticAnalysisTests, MainCheck_ReturnType_Negative) {
    Parser parser("main :: () -> s8;");
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    SymbolAnalysis syma(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(syma));

    SemanticAnalysis sema(opts);
    EXPECT_DEATH(ast->accept(sema), "");
}

TEST_F(SemanticAnalysisTests, ConditionCheck_IfCondition_Positive) {
    Parser parser("foo :: () -> s64 { if 1 { ret 0; } }");
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    SymbolAnalysis syma(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(syma));

    SemanticAnalysis sema(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(sema));
}

TEST_F(SemanticAnalysisTests, ConditionCheck_IfCondition_Negative) {
    Parser parser("bar :: () -> void; foo :: () -> s64 { if bar() { ret 0; } }");
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    SymbolAnalysis syma(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(syma));

    SemanticAnalysis sema(opts);
    EXPECT_DEATH(ast->accept(sema), "");
}

TEST_F(SemanticAnalysisTests, ConditionCheck_UntilCondition_Positive) {
    Parser parser("foo :: () -> s64 { until 1 restart; }");
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    SymbolAnalysis syma(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(syma));

    SemanticAnalysis sema(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(sema));
}

TEST_F(SemanticAnalysisTests, ConditionCheck_UntilCondition_Negative) {
    Parser parser("bar :: () -> void; foo :: () -> s64 { until bar() restart; }");
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    SymbolAnalysis syma(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(syma));

    SemanticAnalysis sema(opts);
    EXPECT_DEATH(ast->accept(sema), "");
}

TEST_F(SemanticAnalysisTests, ControlCheck_StopInLoop_Positive) {
    Parser parser("foo :: () -> s64 { until 1 stop; }");
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    SymbolAnalysis syma(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(syma));

    SemanticAnalysis sema(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(sema));
}

TEST_F(SemanticAnalysisTests, ControlCheck_StopInLoop_Negative) {
    Parser parser("foo :: () -> s64 { stop; }");
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    SymbolAnalysis syma(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(syma));

    SemanticAnalysis sema(opts);
    EXPECT_DEATH(ast->accept(sema), "");
}

TEST_F(SemanticAnalysisTests, ControlCheck_RestartInLoop_Positive) {
    Parser parser("foo :: () -> s64 { until 1 restart; }");
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    SymbolAnalysis syma(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(syma));

    SemanticAnalysis sema(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(sema));
}

TEST_F(SemanticAnalysisTests, ControlCheck_RestartInLoop_Negative) {
    Parser parser("foo :: () -> s64 { restart; }");
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    SymbolAnalysis syma(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(syma));

    SemanticAnalysis sema(opts);
    EXPECT_DEATH(ast->accept(sema), "");
}

TEST_F(SemanticAnalysisTests, TypeCheck_VariableInitializer_Positive) {
    Parser parser("test :: () -> void { let x: s64 = 1; }");
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    SymbolAnalysis syma(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(syma));

    SemanticAnalysis sema(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(sema));
}

TEST_F(SemanticAnalysisTests, TypeCheck_VariableInitializer_Negative) {
    Parser parser("test :: () -> void { let x: s64 = \"test\"; }");
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    SymbolAnalysis syma(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(syma));

    SemanticAnalysis sema(opts);
    EXPECT_DEATH(ast->accept(sema), "");
}

TEST_F(SemanticAnalysisTests, TypeCheck_VoidReturn_Positive) {
    Parser parser("foo :: () -> void { ret; }");
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    SymbolAnalysis syma(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(syma));

    SemanticAnalysis sema(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(sema));
}

TEST_F(SemanticAnalysisTests, TypeCheck_VoidReturn_Negative) {
    Parser parser("foo :: () -> s64 { ret; }");
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    SymbolAnalysis syma(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(syma));

    SemanticAnalysis sema(opts);
    EXPECT_DEATH(ast->accept(sema), "");
}

TEST_F(SemanticAnalysisTests, TypeCheck_Return_Positive) {
    Parser parser("foo :: () -> s64 { ret 1; }");
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    SymbolAnalysis syma(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(syma));

    SemanticAnalysis sema(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(sema));
}

TEST_F(SemanticAnalysisTests, TypeCheck_Return_Negative) {
    Parser parser("foo :: () -> s64 { ret \"test\"; }");
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    SymbolAnalysis syma(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(syma));

    SemanticAnalysis sema(opts);
    EXPECT_DEATH(ast->accept(sema), "");
}

TEST_F(SemanticAnalysisTests, TypeCheck_CastReturn_Positive) {
    Parser parser("foo :: () -> f32 { ret 1; }");
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    SymbolAnalysis syma(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(syma));

    SemanticAnalysis sema(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(sema));
}

TEST_F(SemanticAnalysisTests, Mutability_Assignment_Positive) {
    Parser parser("foo :: () -> s64 { let x: mut s64 = 5; x = 5; ret x; }");
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    SymbolAnalysis syma(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(syma));

    SemanticAnalysis sema(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(sema));
}

TEST_F(SemanticAnalysisTests, Mutability_Assignment_Negative) {
    Parser parser("foo :: () -> s64 { let x: s64 = 5; x = 5; ret x; }");
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    SymbolAnalysis syma(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(syma));

    SemanticAnalysis sema(opts);
    EXPECT_DEATH(ast->accept(sema), "");
}

/*
TEST_F(SemanticAnalysisTests, Mutability_Increment_Positive) {
    Parser parser("foo :: () -> s64 { let x: mut s64 = 5; ret x++; }");
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    SymbolAnalysis syma(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(syma));

    SemanticAnalysis sema(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(sema));
}

TEST_F(SemanticAnalysisTests, Mutability_Decrement_Negative) {
    Parser parser("foo :: () -> s64 { let x: s64 = 5; ret --x; }");
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    SymbolAnalysis syma(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(syma));

    SemanticAnalysis sema(opts);
    EXPECT_DEATH(ast->accept(sema), "");
}
*/

} // namespace lace::test
