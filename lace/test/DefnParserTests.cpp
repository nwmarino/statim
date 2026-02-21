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

TEST_F(DefnParserTests, Functions_Empty) {
    TokenStream stream;
    Lexer lexer("test :: () -> void;");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    ASSERT_NO_FATAL_FAILURE(ast = parser.parse());

    EXPECT_EQ(ast->num_defns(), 1);

    auto F1 = dynamic_cast<const FunctionDefn*>(ast->get_defns()[0]);
    EXPECT_NE(F1, nullptr);
    EXPECT_EQ(F1->name(), "test");
    EXPECT_FALSE(F1->has_runes());
    EXPECT_FALSE(F1->has_params());
    EXPECT_FALSE(F1->has_body());
}

TEST_F(DefnParserTests, Functions_NonEmpty) {
    TokenStream stream;
    Lexer lexer("test :: () -> s64 { ret 0; }");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    ASSERT_NO_FATAL_FAILURE(ast = parser.parse());

    EXPECT_EQ(ast->num_defns(), 1);

    auto F1 = dynamic_cast<const FunctionDefn*>(ast->get_defns()[0]);
    EXPECT_NE(F1, nullptr);
    EXPECT_EQ(F1->name(), "test");
    EXPECT_TRUE(F1->has_body());

    auto B1 = dynamic_cast<const BlockStmt*>(F1->body());
    EXPECT_NE(B1, nullptr);
    EXPECT_EQ(B1->num_stmts(), 1);
    
    auto R1 = dynamic_cast<const RetStmt*>(B1->get_stmt(0));
    EXPECT_NE(R1, nullptr);
    EXPECT_TRUE(R1->has_expr());

    auto I1 = dynamic_cast<const IntegerLiteral*>(R1->expr());
    EXPECT_NE(I1, nullptr);
    EXPECT_EQ(I1->get_value(), 0);
}

TEST_F(DefnParserTests, Functions_WithParameters) {
    TokenStream stream;
    Lexer lexer("test :: (a: s64, b: char) -> void;");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    ASSERT_NO_FATAL_FAILURE(ast = parser.parse());

    EXPECT_EQ(ast->num_defns(), 1);

    auto F1 = dynamic_cast<const FunctionDefn*>(ast->get_defns()[0]);
    EXPECT_NE(F1, nullptr);
    EXPECT_EQ(F1->name(), "test");
    EXPECT_TRUE(F1->has_params());
    EXPECT_EQ(F1->num_params(), 2);

    const ParameterDefn* P1 = F1->get_param(0);
    EXPECT_NE(P1, nullptr);
    EXPECT_EQ(P1->name(), "a");
    EXPECT_EQ(P1->type()->string(), "s64");

    const ParameterDefn* P2 = F1->get_param(1);
    EXPECT_NE(P2, nullptr);
    EXPECT_EQ(P2->name(), "b");
    EXPECT_EQ(P2->type()->string(), "char");
}

TEST_F(DefnParserTests, GlobalVariables_NoInitializer) {
    TokenStream stream;
    Lexer lexer("glob :: s64");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    ASSERT_NO_FATAL_FAILURE(ast = parser.parse());

    EXPECT_EQ(ast->num_defns(), 1);

    auto V1 = dynamic_cast<const VariableDefn*>(ast->get_defns()[0]);
    EXPECT_NE(V1, nullptr);
    EXPECT_EQ(V1->name(), "glob");
    EXPECT_EQ(V1->type()->string(), "s64");
    EXPECT_FALSE(V1->has_init());
}

TEST_F(DefnParserTests, GlobalVariables_WithInitializer) {
    TokenStream stream;
    Lexer lexer("glob :: s64 = 5");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    ASSERT_NO_FATAL_FAILURE(ast = parser.parse());

    EXPECT_EQ(ast->num_defns(), 1);

    auto V1 = dynamic_cast<const VariableDefn*>(ast->get_defns()[0]);
    EXPECT_NE(V1, nullptr);
    EXPECT_EQ(V1->name(), "glob");
    EXPECT_EQ(V1->type()->string(), "s64");
    EXPECT_TRUE(V1->has_init());

    auto I1 = dynamic_cast<const IntegerLiteral*>(V1->init());
    EXPECT_NE(I1, nullptr);
    EXPECT_EQ(I1->get_value(), 5);
}

TEST_F(DefnParserTests, Structs_NonEmpty) {
    TokenStream stream;
    Lexer lexer("Box :: struct { x: s32, y: f32, z: bool }");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    ASSERT_NO_FATAL_FAILURE(ast = parser.parse());

    EXPECT_EQ(ast->num_defns(), 1);

    auto S1 = dynamic_cast<const StructDefn*>(ast->get_defn(0));
    EXPECT_NE(S1, nullptr);
    EXPECT_EQ(S1->name(), "Box");
    EXPECT_EQ(S1->num_fields(), 3);

    auto TY1 = dynamic_cast<const StructType*>(S1->type());
    EXPECT_NE(TY1, nullptr);
    EXPECT_EQ(TY1->defn(), S1);

    const FieldDefn* F1 = S1->get_field("x");
    EXPECT_NE(F1, nullptr);
    EXPECT_EQ(F1, S1->get_field(0));
    EXPECT_EQ(F1->name(), "x");
    EXPECT_EQ(F1->type()->string(), "s32");

    const FieldDefn* F2 = S1->get_field("y");
    EXPECT_NE(F2, nullptr);
    EXPECT_EQ(F2, S1->get_field(1));
    EXPECT_EQ(F2->name(), "y");
    EXPECT_EQ(F2->type()->string(), "f32");

    const FieldDefn* F3 = S1->get_field("z");
    EXPECT_NE(F3, nullptr);
    EXPECT_EQ(F3, S1->get_field(2));
    EXPECT_EQ(F3->name(), "z");
    EXPECT_EQ(F3->type()->string(), "bool");
}

TEST_F(DefnParserTests, Enum) {
    TokenStream stream;
    Lexer lexer("Colors :: enum { Red, Blue = 0, Yellow = -7 }");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    ASSERT_NO_FATAL_FAILURE(ast = parser.parse());

    EXPECT_EQ(ast->num_defns(), 1);

    auto E1 = dynamic_cast<EnumDefn*>(ast->get_defn(0));
    EXPECT_NE(E1, nullptr);
    EXPECT_EQ(E1->name(), "Colors");
    EXPECT_EQ(E1->num_variants(), 3);

    auto ETY = dynamic_cast<const EnumType*>(E1->type());
    EXPECT_NE(ETY, nullptr);
    EXPECT_EQ(ETY->string(), "Colors");

    const Type* underlying = ETY->underlying();
    EXPECT_EQ(underlying->string(), "s64");

    const VariantDefn* V1 = E1->get_variant(0);
    EXPECT_NE(V1, nullptr);
    EXPECT_EQ(V1->name(), "Red");
    EXPECT_EQ(V1->get_value(), 0);

    const VariantDefn* V2 = E1->get_variant(1);
    EXPECT_NE(V2, nullptr);
    EXPECT_EQ(V2->name(), "Blue");
    EXPECT_EQ(V2->get_value(), 0);

    const VariantDefn* V3 = E1->get_variant(2);
    EXPECT_NE(V3, nullptr);
    EXPECT_EQ(V3->name(), "Yellow");
    EXPECT_EQ(V3->get_value(), -7);
}

} // namespace lace::test
