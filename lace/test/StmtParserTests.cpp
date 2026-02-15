//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

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
    Parser parser("test :: () -> s64 { if 5 { ret 0; } }");
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    EXPECT_EQ(ast->num_defns(), 1);

    FunctionDefn* FD = dynamic_cast<FunctionDefn*>(ast->get_defns()[0]);
    EXPECT_NE(FD, nullptr);
    EXPECT_TRUE(FD->has_body());

    BlockStmt* BS = dynamic_cast<BlockStmt*>(FD->get_body());
    EXPECT_NE(BS, nullptr);
    EXPECT_EQ(BS->num_stmts(), 1);

    IfStmt* IS = dynamic_cast<IfStmt*>(BS->get_stmt(0));
    EXPECT_NE(IS, nullptr);
    EXPECT_FALSE(IS->has_else());

    IntegerLiteral* IL = dynamic_cast<IntegerLiteral*>(IS->get_cond());
    EXPECT_NE(IL, nullptr);
    EXPECT_EQ(IL->get_value(), 5);

    BlockStmt* BS2 = dynamic_cast<BlockStmt*>(IS->get_then());
    EXPECT_NE(BS2, nullptr);
    EXPECT_EQ(BS2->num_stmts(), 1);

    RetStmt* RS = dynamic_cast<RetStmt*>(BS2->get_stmt(0));
    EXPECT_NE(RS, nullptr);
}

TEST_F(StmtParserTests, IfElseStatement_Positive) {
    Parser parser("test :: () -> s64 { if 5 ret 0; else ret 1; }");
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    EXPECT_EQ(ast->num_defns(), 1);

    FunctionDefn* FD = dynamic_cast<FunctionDefn*>(ast->get_defns()[0]);
    EXPECT_NE(FD, nullptr);
    EXPECT_TRUE(FD->has_body());

    BlockStmt* BS = dynamic_cast<BlockStmt*>(FD->get_body());
    EXPECT_NE(BS, nullptr);
    EXPECT_EQ(BS->num_stmts(), 1);

    IfStmt* IS = dynamic_cast<IfStmt*>(BS->get_stmt(0));
    EXPECT_NE(IS, nullptr);
    EXPECT_TRUE(IS->has_else());

    IntegerLiteral* IL = dynamic_cast<IntegerLiteral*>(IS->get_cond());
    EXPECT_NE(IL, nullptr);
    EXPECT_EQ(IL->get_value(), 5);

    RetStmt* RS = dynamic_cast<RetStmt*>(IS->get_then());
    EXPECT_NE(RS, nullptr);

    RetStmt* RS2 = dynamic_cast<RetStmt*>(IS->get_else());
    EXPECT_NE(RS2, nullptr);
}

TEST_F(StmtParserTests, UntilStatement_Positive) {
    Parser parser("test :: () -> s64 { until 1 restart; }");
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    EXPECT_EQ(ast->num_defns(), 1);

    FunctionDefn* FD = dynamic_cast<FunctionDefn*>(ast->get_defns()[0]);
    EXPECT_NE(FD, nullptr);
    EXPECT_TRUE(FD->has_body());

    BlockStmt* BS = dynamic_cast<BlockStmt*>(FD->get_body());
    EXPECT_NE(BS, nullptr);
    EXPECT_EQ(BS->num_stmts(), 1);

    UntilStmt* US = dynamic_cast<UntilStmt*>(BS->get_stmt(0));
    EXPECT_NE(US, nullptr);
    EXPECT_TRUE(US->has_body());

    RestartStmt* CS = dynamic_cast<RestartStmt*>(US->get_body());
    EXPECT_NE(CS, nullptr);
}

TEST_F(StmtParserTests, UntilStatementNoBody_Positive) {
    Parser parser("test :: () -> s64 { until 1; }");
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    EXPECT_EQ(ast->num_defns(), 1);

    FunctionDefn* FD = dynamic_cast<FunctionDefn*>(ast->get_defns()[0]);
    EXPECT_NE(FD, nullptr);
    EXPECT_TRUE(FD->has_body());

    BlockStmt* BS = dynamic_cast<BlockStmt*>(FD->get_body());
    EXPECT_NE(BS, nullptr);
    EXPECT_EQ(BS->num_stmts(), 1);

    UntilStmt* US = dynamic_cast<UntilStmt*>(BS->get_stmt(0));
    EXPECT_NE(US, nullptr);
    EXPECT_FALSE(US->has_body());
}

/*
TEST_F(StmtParserTests, AsmStatement_Positive) {
    Parser parser("test :: () -> void { asm {\"movq ^0, %rax\n\" \"syscall\n\" : : \"r\" (x) : \"rax\"}; }");
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    EXPECT_EQ(ast->num_defns(), 1);

    FunctionDefn* FD = dynamic_cast<FunctionDefn*>(ast->get_defns()[0]);
    EXPECT_NE(FD, nullptr);
    EXPECT_TRUE(FD->has_body());

    BlockStmt* BS = dynamic_cast<BlockStmt*>(FD->get_body());
    EXPECT_NE(BS, nullptr);
    EXPECT_EQ(BS->num_stmts(), 1);

    AsmStmt* AS = dynamic_cast<AsmStmt*>(BS->get_stmt(0));
    EXPECT_NE(AS, nullptr);
    EXPECT_EQ(AS->num_output_constraints(), 0);
    EXPECT_EQ(AS->num_input_constraints(), 1);
    EXPECT_EQ(AS->num_args(), 1);
    EXPECT_EQ(AS->num_clobbers(), 1);
    
    EXPECT_EQ(AS->get_input_constraint(0), "r");
    EXPECT_EQ(AS->get_clobber(0), "rax");

    DeclRefExpr* A1 = dynamic_cast<DeclRefExpr*>(AS->get_arg(0));
    EXPECT_NE(A1, nullptr);
    EXPECT_EQ(A1->get_name(), "x");   
}
*/

} // namespace lace::test
