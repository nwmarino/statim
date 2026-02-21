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

class StmtParserTests : public ::testing::Test {
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

TEST_F(StmtParserTests, IfStatement_Positive) {
    TokenStream stream;
    Lexer lexer("test :: () -> s64 { if 5 { ret 0; } }");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    ASSERT_NO_FATAL_FAILURE(ast = parser.parse());

    EXPECT_EQ(ast->num_defns(), 1);

    FunctionDefn* FD = dynamic_cast<FunctionDefn*>(ast->get_defn(0));
    EXPECT_NE(FD, nullptr);
    EXPECT_TRUE(FD->has_body());

    BlockStmt* BS = dynamic_cast<BlockStmt*>(FD->body());
    EXPECT_NE(BS, nullptr);
    EXPECT_EQ(BS->num_stmts(), 1);

    IfStmt* IS = dynamic_cast<IfStmt*>(BS->get_stmt(0));
    EXPECT_NE(IS, nullptr);
    EXPECT_FALSE(IS->has_else());

    IntegerLiteral* IL = dynamic_cast<IntegerLiteral*>(IS->condition());
    EXPECT_NE(IL, nullptr);
    EXPECT_EQ(IL->get_value(), 5);

    BlockStmt* BS2 = dynamic_cast<BlockStmt*>(IS->then_body());
    EXPECT_NE(BS2, nullptr);
    EXPECT_EQ(BS2->num_stmts(), 1);

    RetStmt* RS = dynamic_cast<RetStmt*>(BS2->get_stmt(0));
    EXPECT_NE(RS, nullptr);
}

TEST_F(StmtParserTests, IfElseStatement_Positive) {
    TokenStream stream;
    Lexer lexer("test :: () -> s64 { if 5 ret 0; else ret 1; }");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    ASSERT_NO_FATAL_FAILURE(ast = parser.parse());

    EXPECT_EQ(ast->num_defns(), 1);

    FunctionDefn* FD = dynamic_cast<FunctionDefn*>(ast->get_defn(0));
    EXPECT_NE(FD, nullptr);
    EXPECT_TRUE(FD->has_body());

    BlockStmt* BS = dynamic_cast<BlockStmt*>(FD->body());
    EXPECT_NE(BS, nullptr);
    EXPECT_EQ(BS->num_stmts(), 1);

    IfStmt* IS = dynamic_cast<IfStmt*>(BS->get_stmt(0));
    EXPECT_NE(IS, nullptr);
    EXPECT_TRUE(IS->has_else());

    IntegerLiteral* IL = dynamic_cast<IntegerLiteral*>(IS->condition());
    EXPECT_NE(IL, nullptr);
    EXPECT_EQ(IL->get_value(), 5);

    RetStmt* RS = dynamic_cast<RetStmt*>(IS->then_body());
    EXPECT_NE(RS, nullptr);

    RetStmt* RS2 = dynamic_cast<RetStmt*>(IS->else_body());
    EXPECT_NE(RS2, nullptr);
}

TEST_F(StmtParserTests, UntilStatement_Positive) {
    TokenStream stream;
    Lexer lexer("test :: () -> s64 { until 1 restart; }");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    ASSERT_NO_FATAL_FAILURE(ast = parser.parse());

    EXPECT_EQ(ast->num_defns(), 1);

    FunctionDefn* FD = dynamic_cast<FunctionDefn*>(ast->get_defn(0));
    EXPECT_NE(FD, nullptr);
    EXPECT_TRUE(FD->has_body());

    BlockStmt* BS = dynamic_cast<BlockStmt*>(FD->body());
    EXPECT_NE(BS, nullptr);
    EXPECT_EQ(BS->num_stmts(), 1);

    UntilStmt* US = dynamic_cast<UntilStmt*>(BS->get_stmt(0));
    EXPECT_NE(US, nullptr);
    EXPECT_TRUE(US->has_body());

    RestartStmt* CS = dynamic_cast<RestartStmt*>(US->body());
    EXPECT_NE(CS, nullptr);
}

TEST_F(StmtParserTests, UntilStatementNoBody_Positive) {
    TokenStream stream;
    Lexer lexer("test :: () -> s64 { until 1; }");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    ASSERT_NO_FATAL_FAILURE(ast = parser.parse());

    EXPECT_EQ(ast->num_defns(), 1);

    FunctionDefn* FD = dynamic_cast<FunctionDefn*>(ast->get_defn(0));
    EXPECT_NE(FD, nullptr);
    EXPECT_TRUE(FD->has_body());

    BlockStmt* BS = dynamic_cast<BlockStmt*>(FD->body());
    EXPECT_NE(BS, nullptr);
    EXPECT_EQ(BS->num_stmts(), 1);

    UntilStmt* US = dynamic_cast<UntilStmt*>(BS->get_stmt(0));
    EXPECT_NE(US, nullptr);
    EXPECT_FALSE(US->has_body());
}

} // namespace lace::test
