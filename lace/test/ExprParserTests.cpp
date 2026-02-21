//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/lexer/Lexer.h"
#include "lace/lexer/TokenStream.h"
#include "lace/parser/Parser.h"
#include "lace/tree/AST.h"
#include "lace/tree/Expr.h"
#include "lace/tree/Stmt.h"

#include "gtest/gtest.h"

namespace lace::test {

class ExprParserTests : public ::testing::Test {
protected:
    AST* ast;

    void SetUp() override {
        ast = nullptr;
    }

    void TearDown() override {
        if (ast)
            delete ast;
            
        ast = nullptr;
    }
};

TEST_F(ExprParserTests, StructInitExpr) {
    TokenStream stream;
    Lexer lexer("test :: () -> void { Vec3 { a: 1, b: 2, c: 3 }; }");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    ASSERT_NO_FATAL_FAILURE(ast = parser.parse());

    EXPECT_EQ(ast->num_defns(), 1);

    auto F = dynamic_cast<FunctionDefn*>(ast->get_defns()[0]);
    EXPECT_NE(F, nullptr);
    EXPECT_TRUE(F->has_body());

    auto B = dynamic_cast<BlockStmt*>(F->body());
    EXPECT_NE(B, nullptr);
    EXPECT_EQ(B->num_stmts(), 1);

    auto A = dynamic_cast<const AdapterStmt*>(B->get_stmt(0));
    EXPECT_NE(A, nullptr);
    EXPECT_TRUE(A->is_expressive());

    auto SI = dynamic_cast<const StructInitExpr*>(A->expr());
    EXPECT_NE(SI, nullptr);
    EXPECT_FALSE(SI->empty());
    EXPECT_EQ(SI->num_fields(), 3);

    auto F1 = dynamic_cast<const IntegerLiteral*>(SI->get_field("a"));
    EXPECT_NE(F1, nullptr);
    EXPECT_EQ(F1->get_value(), 1);

    auto F2 = dynamic_cast<const IntegerLiteral*>(SI->get_field("b"));
    EXPECT_NE(F2, nullptr);
    EXPECT_EQ(F2->get_value(), 2);
    
    auto F3 = dynamic_cast<const IntegerLiteral*>(SI->get_field("c"));
    EXPECT_NE(F3, nullptr);
    EXPECT_EQ(F3->get_value(), 3);
}

} // namespace lace::test
