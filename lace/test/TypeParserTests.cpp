//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

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
    Parser parser("test :: () -> s64;");
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

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
    Parser parser("test :: () -> *bool;");
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

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

TEST_F(TypeParserTests, MutableType) {
    Parser parser("test :: () -> mut void;");
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    EXPECT_EQ(ast->num_defns(), 1);

    FunctionDefn* FD = dynamic_cast<FunctionDefn*>(ast->get_defns()[0]);
    EXPECT_NE(FD, nullptr);

    const QualType& return_type = FD->get_return_type();
    EXPECT_TRUE(return_type.isMut());
    EXPECT_EQ(return_type->string(), "mut void");

    const BuiltinType* BT = dynamic_cast<const BuiltinType*>(return_type.getType());
    EXPECT_NE(BT, nullptr);
    EXPECT_EQ(BT->kind(), BuiltinType::Kind::Void);
}

TEST_F(TypeParserTests, MutablePointerToVoidType) {
    Parser parser("test :: () -> mut *void;");
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    EXPECT_EQ(ast->num_defns(), 1);

    FunctionDefn* FD = dynamic_cast<FunctionDefn*>(ast->get_defns()[0]);
    EXPECT_NE(FD, nullptr);

    const QualType& return_type = FD->get_return_type();
    EXPECT_TRUE(return_type.isMut());
    EXPECT_EQ(return_type->string(), "mut *void");

    const PointerType* PT = dynamic_cast<const PointerType*>(return_type.getType());
    EXPECT_NE(PT, nullptr);
    EXPECT_EQ(PT->string(), "*void");

    const QualType& pointee = PT->pointee();
    EXPECT_FALSE(pointee.isMut());
    EXPECT_EQ(pointee.string(), "void");

    const BuiltinType* BT = dynamic_cast<const BuiltinType*>(pointee.getType());
    EXPECT_NE(BT, nullptr);
    EXPECT_EQ(BT->kind(), BuiltinType::Kind::Void);
}

TEST_F(TypeParserTests, PointerToMutableVoidType) {
    Parser parser("test :: () -> *mut void;");
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    EXPECT_EQ(ast->num_defns(), 1);

    FunctionDefn* FD = dynamic_cast<FunctionDefn*>(ast->get_defns()[0]);
    EXPECT_NE(FD, nullptr);

    const QualType& return_type = FD->get_return_type();
    EXPECT_FALSE(return_type.isMut());
    EXPECT_EQ(return_type.string(), "*mut void");

    const PointerType* PT = dynamic_cast<const PointerType*>(return_type.getType());
    EXPECT_NE(PT, nullptr);
    EXPECT_EQ(PT->string(), "*mut void");

    const QualType& pointee = PT->pointee();
    EXPECT_TRUE(pointee.isMut());
    EXPECT_EQ(pointee.string(), "mut void");

    const BuiltinType* BT = dynamic_cast<const BuiltinType*>(pointee.getType());
    EXPECT_NE(BT, nullptr);
    EXPECT_EQ(BT->kind(), BuiltinType::Kind::Void);
}

TEST_F(TypeParserTests, MutablePointerToMutableVoidType) {
    Parser parser("test :: () -> mut *mut void;");
    EXPECT_NO_FATAL_FAILURE(ast = parser.parse());

    EXPECT_EQ(ast->num_defns(), 1);

    FunctionDefn* FD = dynamic_cast<FunctionDefn*>(ast->get_defns()[0]);
    EXPECT_NE(FD, nullptr);

    const QualType& return_type = FD->get_return_type();
    EXPECT_TRUE(return_type.isMut());
    EXPECT_EQ(return_type.string(), "mut *mut void");

    const PointerType* PT = dynamic_cast<const PointerType*>(return_type.getType());
    EXPECT_NE(PT, nullptr);
    EXPECT_EQ(PT->string(), "*mut void");

    const QualType& pointee = PT->pointee();
    EXPECT_TRUE(pointee.isMut());
    EXPECT_EQ(pointee.string(), "mut void");

    const BuiltinType* BT = dynamic_cast<const BuiltinType*>(pointee.getType());
    EXPECT_NE(BT, nullptr);
    EXPECT_EQ(BT->kind(), BuiltinType::Kind::Void);
}

} // namespace lace::test
