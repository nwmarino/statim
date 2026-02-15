//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

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
        if (ast) {
            delete ast;
            ast = nullptr;
        }
    }
};

TEST_F(SymbolAnalysisTests, VariableRef_Positive) {
    Parser parser("test :: () -> s64 { let x: s64 = 0; ret x; }");
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    SymbolAnalysis syma(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(syma));
}

TEST_F(SymbolAnalysisTests, VariableRef_Negative) {
    Parser parser("test :: () -> s64 { let x: s64 = 0; ret y; }");
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    SymbolAnalysis syma(opts);
    EXPECT_DEATH(ast->accept(syma), "");
}

TEST_F(SymbolAnalysisTests, CalleeRef_Positive) {
    Parser parser("foo :: () -> s64 { ret bar(); } bar :: () -> s64 { ret 0; }");
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    SymbolAnalysis syma(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(syma));
}

TEST_F(SymbolAnalysisTests, ParamRef_Positive) {
    Parser parser("foo :: (a: s64) -> s64 { ret a; }");
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    SymbolAnalysis syma(opts);
    EXPECT_NO_FATAL_FAILURE(ast->accept(syma));
}

} // namespace lace::test
