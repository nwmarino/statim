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
#include "lace/tree/Type.h"

#include "gtest/gtest.h"

namespace lace::test {

class DefnParserTests : public ::testing::Test {
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

TEST_F(DefnParserTests, EmptyFunction) {
    TokenStream stream;
    Lexer lexer("test :: () -> void;");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    EXPECT_EQ(ast->num_defns(), 1);

    const FunctionDefn* FD = dynamic_cast<const FunctionDefn*>(ast->get_defns()[0]);
    EXPECT_NE(FD, nullptr);
    EXPECT_EQ(FD->get_name(), "test");
    EXPECT_FALSE(FD->has_runes());
    EXPECT_FALSE(FD->has_params());
    EXPECT_FALSE(FD->has_body());
}

TEST_F(DefnParserTests, FunctionWithBody) {
    TokenStream stream;
    Lexer lexer("test :: () -> s64 { ret 0; }");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    EXPECT_EQ(ast->num_defns(), 1);

    const FunctionDefn* FD = dynamic_cast<const FunctionDefn*>(ast->get_defns()[0]);
    EXPECT_NE(FD, nullptr);
    EXPECT_EQ(FD->get_name(), "test");
    EXPECT_TRUE(FD->has_body());

    const BlockStmt* BS = dynamic_cast<const BlockStmt*>(FD->get_body());
    EXPECT_NE(BS, nullptr);
    EXPECT_EQ(BS->num_stmts(), 1);
    
    const RetStmt* RS = dynamic_cast<const RetStmt*>(BS->get_stmt(0));
    EXPECT_NE(RS, nullptr);
    EXPECT_TRUE(RS->has_expr());

    const IntegerLiteral* IL = dynamic_cast<const IntegerLiteral*>(RS->get_expr());
    EXPECT_NE(IL, nullptr);
    EXPECT_EQ(IL->get_value(), 0);
}

TEST_F(DefnParserTests, FunctionParameters) {
    TokenStream stream;
    Lexer lexer("test :: (a: s64, b: char) -> void;");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    EXPECT_EQ(ast->num_defns(), 1);

    const FunctionDefn* FD = dynamic_cast<FunctionDefn*>(ast->get_defns()[0]);
    EXPECT_NE(FD, nullptr);
    EXPECT_EQ(FD->get_name(), "test");
    EXPECT_TRUE(FD->has_params());
    EXPECT_EQ(FD->num_params(), 2);

    const ParameterDefn* P1 = FD->get_param(0);
    EXPECT_NE(P1, nullptr);
    EXPECT_EQ(P1->get_name(), "a");
    EXPECT_EQ(P1->get_type()->string(), "s64");

    const ParameterDefn* P2 = FD->get_param(1);
    EXPECT_NE(P2, nullptr);
    EXPECT_EQ(P2->get_name(), "b");
    EXPECT_EQ(P2->get_type().string(), "char");
}

TEST_F(DefnParserTests, Global) {
    TokenStream stream;
    Lexer lexer("glob :: s64");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    EXPECT_EQ(ast->num_defns(), 1);

    const VariableDefn* VD = dynamic_cast<VariableDefn*>(ast->get_defns()[0]);
    EXPECT_NE(VD, nullptr);
    EXPECT_EQ(VD->get_name(), "glob");
    EXPECT_EQ(VD->get_type().string(), "s64");
    EXPECT_FALSE(VD->has_init());
}

TEST_F(DefnParserTests, GlobalWithInitializer) {
    TokenStream stream;
    Lexer lexer("glob :: s64 = 5");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    EXPECT_EQ(ast->num_defns(), 1);

    const VariableDefn* VD = dynamic_cast<VariableDefn*>(ast->get_defns()[0]);
    EXPECT_NE(VD, nullptr);
    EXPECT_EQ(VD->get_name(), "glob");
    EXPECT_EQ(VD->get_type().string(), "s64");
    EXPECT_TRUE(VD->has_init());

    const IntegerLiteral* IL = dynamic_cast<const IntegerLiteral*>(VD->get_init());
    EXPECT_NE(IL, nullptr);
    EXPECT_EQ(IL->get_value(), 5);
}

TEST_F(DefnParserTests, Struct) {
    TokenStream stream;
    Lexer lexer("Box :: struct { x: s32, y: f32, z: bool }");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    EXPECT_EQ(ast->num_defns(), 1);

    const StructDefn* SD = dynamic_cast<const StructDefn*>(ast->get_defn(0));
    EXPECT_NE(SD, nullptr);
    EXPECT_EQ(SD->get_name(), "Box");
    EXPECT_EQ(SD->num_fields(), 3);

    const StructType* ST = dynamic_cast<const StructType*>(SD->get_type());
    EXPECT_NE(ST, nullptr);
    EXPECT_EQ(ST->getDefn(), SD);

    const FieldDefn* F1 = SD->get_field("x");
    EXPECT_NE(F1, nullptr);
    EXPECT_EQ(F1, SD->get_field(0));
    EXPECT_EQ(F1->get_name(), "x");
    EXPECT_EQ(F1->get_type()->string(), "s32");

    const FieldDefn* F2 = SD->get_field("y");
    EXPECT_NE(F2, nullptr);
    EXPECT_EQ(F2, SD->get_field(1));
    EXPECT_EQ(F2->get_name(), "y");
    EXPECT_EQ(F2->get_type().string(), "f32");

    const FieldDefn* F3 = SD->get_field("z");
    EXPECT_NE(F3, nullptr);
    EXPECT_EQ(F3, SD->get_field(2));
    EXPECT_EQ(F3->get_name(), "z");
    EXPECT_EQ(F3->get_type().string(), "bool");
}

TEST_F(DefnParserTests, Enum) {
    TokenStream stream;
    Lexer lexer("Colors :: enum { Red, Blue = 0, Yellow = -7 }");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    EXPECT_EQ(ast->num_defns(), 1);

    const EnumDefn* ED = dynamic_cast<EnumDefn*>(ast->get_defn(0));
    EXPECT_NE(ED, nullptr);
    EXPECT_EQ(ED->get_name(), "Colors");
    EXPECT_EQ(ED->num_variants(), 3);

    const EnumType* ET = dynamic_cast<const EnumType*>(ED->get_type());
    EXPECT_NE(ET, nullptr);
    EXPECT_EQ(ET->string(), "Colors");

    const QualType& underlying = ET->underlying();
    EXPECT_EQ(underlying.string(), "s64");

    const VariantDefn* V1 = ED->get_variant(0);
    EXPECT_NE(V1, nullptr);
    EXPECT_EQ(V1->get_name(), "Red");
    EXPECT_EQ(V1->get_value(), 0);

    const VariantDefn* V2 = ED->get_variant(1);
    EXPECT_NE(V2, nullptr);
    EXPECT_EQ(V2->get_name(), "Blue");
    EXPECT_EQ(V2->get_value(), 0);

    const VariantDefn* V3 = ED->get_variant(2);
    EXPECT_NE(V3, nullptr);
    EXPECT_EQ(V3->get_name(), "Yellow");
    EXPECT_EQ(V3->get_value(), -7);
}

} // namespace lace::test
