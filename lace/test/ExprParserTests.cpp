//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/lexer/Lexer.h"
#include "lace/parser/Parser.h"
#include "lace/tree/AST.h"
#include "lace/tree/Defn.h"
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
        if (ast) { 
            delete ast;
            ast = nullptr;
        }
    }
};

/*
TEST_F(ExprParserTests, IntegerLiteral_TypeSuffixes) {
    Parser parser("test :: () -> void { 1b; 2ub; 3s; 4us; 5i; 6ui; 7l; 8ul; }");
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    EXPECT_EQ(ast->num_defns(), 1);

    FunctionDefn* FD = dynamic_cast<FunctionDefn*>(ast->get_defns()[0]);
    EXPECT_NE(FD, nullptr);
    EXPECT_TRUE(FD->has_body());

    BlockStmt* BS = dynamic_cast<BlockStmt*>(FD->get_body());
    EXPECT_NE(BS, nullptr);
    EXPECT_EQ(BS->num_stmts(), 8);

    IntegerLiteral* IL1 = dynamic_cast<IntegerLiteral*>(BS->get_stmt(0));
    EXPECT_NE(IL1, nullptr);
    EXPECT_EQ(IL1->get_value(), 1);
    EXPECT_EQ(IL1->get_type().to_string(), "s8");

    IntegerLiteral* IL2 = dynamic_cast<IntegerLiteral*>(BS->get_stmt(1));
    EXPECT_NE(IL2, nullptr);
    EXPECT_EQ(IL2->get_value(), 2);
    EXPECT_EQ(IL2->get_type().to_string(), "u8");

    IntegerLiteral* IL3 = dynamic_cast<IntegerLiteral*>(BS->get_stmt(2));
    EXPECT_NE(IL3, nullptr);
    EXPECT_EQ(IL3->get_value(), 3);
    EXPECT_EQ(IL3->get_type().to_string(), "s16");

    IntegerLiteral* IL4 = dynamic_cast<IntegerLiteral*>(BS->get_stmt(3));
    EXPECT_NE(IL4, nullptr);
    EXPECT_EQ(IL4->get_value(), 4);
    EXPECT_EQ(IL4->get_type().to_string(), "u16");

    IntegerLiteral* IL5 = dynamic_cast<IntegerLiteral*>(BS->get_stmt(4));
    EXPECT_NE(IL5, nullptr);
    EXPECT_EQ(IL5->get_value(), 5);
    EXPECT_EQ(IL5->get_type().to_string(), "s32");

    IntegerLiteral* IL6 = dynamic_cast<IntegerLiteral*>(BS->get_stmt(5));
    EXPECT_NE(IL6, nullptr);
    EXPECT_EQ(IL6->get_value(), 6);
    EXPECT_EQ(IL6->get_type().to_string(), "u32");

    IntegerLiteral* IL7 = dynamic_cast<IntegerLiteral*>(BS->get_stmt(6));
    EXPECT_NE(IL7, nullptr);
    EXPECT_EQ(IL7->get_value(), 7);
    EXPECT_EQ(IL7->get_type().to_string(), "s64");

    IntegerLiteral* IL8 = dynamic_cast<IntegerLiteral*>(BS->get_stmt(7));
    EXPECT_NE(IL8, nullptr);
    EXPECT_EQ(IL8->get_value(), 8);
    EXPECT_EQ(IL8->get_type().to_string(), "u64");
}
*/

TEST_F(ExprParserTests, FloatLiteral_TypeSuffixes) {
    TokenStream stream;
    Lexer lexer("test :: () -> void { 1.f; 2.d; }");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    EXPECT_EQ(ast->num_defns(), 1);

    const FunctionDefn* FD = dynamic_cast<const FunctionDefn*>(ast->get_defns()[0]);
    EXPECT_NE(FD, nullptr);
    EXPECT_TRUE(FD->has_body());

    const BlockStmt* BS = dynamic_cast<const BlockStmt*>(FD->get_body());
    EXPECT_NE(BS, nullptr);
    EXPECT_EQ(BS->num_stmts(), 2);

    const AdapterStmt* AS = dynamic_cast<const AdapterStmt*>(BS->get_stmt(0));
    EXPECT_NE(AS, nullptr);
    EXPECT_EQ(AS->get_flavor(), AdapterStmt::Expressive);

    const FloatLiteral* FL = dynamic_cast<const FloatLiteral*>(AS->get_expr());
    EXPECT_NE(FL, nullptr);
    EXPECT_EQ(FL->get_type().string(), "f32");

    AS = dynamic_cast<const AdapterStmt*>(BS->get_stmt(1));
    EXPECT_NE(AS, nullptr);
    EXPECT_EQ(AS->get_flavor(), AdapterStmt::Expressive);

    FL = dynamic_cast<const FloatLiteral*>(AS->get_expr());
    EXPECT_NE(FL, nullptr);
    EXPECT_EQ(FL->get_type().string(), "f64");
}

} // namespace lace::test
