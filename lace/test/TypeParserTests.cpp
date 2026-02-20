//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/lexer/Lexer.h"
#include "lace/parser/Parser.h"
#include "lace/tree/AST.h"
#include "lace/tree/Defn.h"
#include "lace/tree/Type.h"

#include "gtest/gtest.h"

namespace lace::test {

class TypeParserTests : public ::testing::Test {
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

TEST_F(TypeParserTests, BuiltinType) {
    TokenStream stream;
    Lexer lexer("test :: () -> s64;");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    ASSERT_NO_FATAL_FAILURE(ast = parser.parse());

    EXPECT_EQ(ast->num_defns(), 1);

    FunctionDefn* FD = dynamic_cast<FunctionDefn*>(ast->get_defns()[0]);
    EXPECT_NE(FD, nullptr);

    const QualType& return_type = FD->get_return_type();
    EXPECT_EQ(return_type.string(), "s64");

    const BuiltinType* BT = dynamic_cast<const BuiltinType*>(return_type.getType());
    EXPECT_NE(BT, nullptr);
    EXPECT_EQ(BT->kind(), BuiltinType::Kind::Int64);
}

TEST_F(TypeParserTests, PointerType) {
    TokenStream stream;
    Lexer lexer("test :: () -> *bool;");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    ASSERT_NO_FATAL_FAILURE(ast = parser.parse());

    EXPECT_EQ(ast->num_defns(), 1);

    FunctionDefn* FD = dynamic_cast<FunctionDefn*>(ast->get_defns()[0]);
    EXPECT_NE(FD, nullptr);

    const QualType& return_type = FD->get_return_type();
    EXPECT_EQ(return_type->string(), "*bool");

    const PointerType* PT = dynamic_cast<const PointerType*>(return_type.getType());
    EXPECT_NE(PT, nullptr);
    
    const QualType& pointee = PT->pointee();
    EXPECT_EQ(pointee->string(), "bool");

    const BuiltinType* BT = dynamic_cast<const BuiltinType*>(pointee.getType());
    EXPECT_NE(BT, nullptr);
    EXPECT_EQ(BT->kind(), BuiltinType::Kind::Bool);
}

} // namespace lace::test
