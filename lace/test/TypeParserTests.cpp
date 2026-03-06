//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/lexer/Lexer.h"
#include "lace/parser/Parser.h"
#include "lace/tree/Rib.h"
#include "lace/tree/Defn.h"
#include "lace/tree/Type.h"

#include "gtest/gtest.h"

namespace lace::test {

class TypeParserTests : public ::testing::Test {
protected:
    Rib* rib;

    void SetUp() override {
        rib = nullptr;
    }

    void TearDown() override {
        if (rib)
            delete rib;

        rib = nullptr;
    }
};

TEST_F(TypeParserTests, BuiltinType) {
    TokenStream stream;
    Lexer lexer("test :: () -> s64;");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    ASSERT_NO_FATAL_FAILURE(rib = parser.parse());

    EXPECT_EQ(rib->num_defns(), 1);

    FunctionDefn* FD = dynamic_cast<FunctionDefn*>(rib->get_defn(0));
    EXPECT_NE(FD, nullptr);

    const BuiltinType* BT = dynamic_cast<const BuiltinType*>(FD->get_return_type());
    EXPECT_NE(BT, nullptr);
    EXPECT_EQ(BT->kind(), BuiltinType::Kind::Int64);
}

TEST_F(TypeParserTests, PointerType) {
    TokenStream stream;
    Lexer lexer("test :: () -> *bool;");
    ASSERT_TRUE(lexer.lex(stream));

    Parser parser(stream);
    ASSERT_NO_FATAL_FAILURE(rib = parser.parse());

    EXPECT_EQ(rib->num_defns(), 1);

    FunctionDefn* FD = dynamic_cast<FunctionDefn*>(rib->get_defn(0));
    EXPECT_NE(FD, nullptr);

    const PointerType* PT = dynamic_cast<const PointerType*>(FD->get_return_type());
    EXPECT_NE(PT, nullptr);

    const BuiltinType* BT = dynamic_cast<const BuiltinType*>(PT->pointee());
    EXPECT_NE(BT, nullptr);
    EXPECT_EQ(BT->kind(), BuiltinType::Kind::Bool);
}

} // namespace lace::test
