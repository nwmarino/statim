//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/lexer/Lexer.h"
#include "lace/parser/Parser.h"
#include "lace/tree/Rib.h"
#include "lace/tree/SymbolAnalysis.h"

#include "gtest/gtest.h"

namespace lace::test {

class SymbolAnalysisTests : public ::testing::Test {
protected:
    Options opts;
    Context context;
    Rib* rib;

    void SetUp() override {
        opts = {};
        rib = nullptr;
    }

    void TearDown() override {
        if (rib)
            delete rib;
            
        rib = nullptr;
    }
};

TEST_F(SymbolAnalysisTests, VariableRef_Positive) {
    TokenStream stream;
    Lexer lexer("test :: () -> s64 { let x: s64 = 0; ret x; }");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    ASSERT_NO_FATAL_FAILURE(rib = parser.parse());

    SymbolAnalysis syma(context);
    EXPECT_NO_FATAL_FAILURE(rib->accept(syma));
}

TEST_F(SymbolAnalysisTests, VariableRef_Negative) {
    TokenStream stream;
    Lexer lexer("test :: () -> s64 { let x: s64 = 0; ret y; }");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    ASSERT_NO_FATAL_FAILURE(rib = parser.parse());

    SymbolAnalysis syma(context);
    EXPECT_DEATH(rib->accept(syma), "");
}

TEST_F(SymbolAnalysisTests, CalleeRef_Positive) {
    TokenStream stream;
    Lexer lexer("foo :: () -> s64 { ret bar(); } bar :: () -> s64 { ret 0; }");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    ASSERT_NO_FATAL_FAILURE(rib = parser.parse());

    SymbolAnalysis syma(context);
    EXPECT_NO_FATAL_FAILURE(rib->accept(syma));
}

TEST_F(SymbolAnalysisTests, ParamRef_Positive) {
    TokenStream stream;
    Lexer lexer("foo :: (a: s64) -> s64 { ret a; }");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    ASSERT_NO_FATAL_FAILURE(rib = parser.parse());

    SymbolAnalysis syma(context);
    EXPECT_NO_FATAL_FAILURE(rib->accept(syma));
}

} // namespace lace::test
