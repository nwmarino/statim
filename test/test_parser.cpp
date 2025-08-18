#include "ast.hpp"
#include "parser.hpp"
#include "translation_unit.hpp"

#include <gtest/gtest.h>

namespace stm {

namespace test {

class ParserTest : public ::testing::Test {};

TEST_F(ParserTest, parse_function_basic) {
    InputFile file;
    file.pPath = "test";
    file.overwrite("main :: () -> void {}");

    TranslationUnit unit { file };
    Parser parser { file };
    parser.parse(unit);
    
    Root& root = unit.get_root();
    EXPECT_EQ(root.num_decls(), 1);

    auto decl = root.get_decls()[0];
    EXPECT_NE(decl, nullptr);

    auto function = dynamic_cast<FunctionDecl*>(decl);
    EXPECT_NE(function, nullptr);
    EXPECT_EQ(function->get_name(), "main");
    EXPECT_EQ(function->num_params(), 0);
    
    auto body = function->get_body();
    EXPECT_NE(body, nullptr);

    auto block = dynamic_cast<const BlockStmt*>(body);
    EXPECT_NE(block, nullptr);
    EXPECT_EQ(block->size(), 0);
}

TEST_F(ParserTest, parse_return_basic) {
    InputFile file;
    file.pPath = "test";
    file.overwrite("main :: () -> void { ret 42; }");

    TranslationUnit unit { file };
    Parser parser { file };
    parser.parse(unit);
    
    Root& root = unit.get_root();
    EXPECT_EQ(root.num_decls(), 1);

    auto decl = root.get_decls()[0];
    EXPECT_NE(decl, nullptr);

    auto function = dynamic_cast<FunctionDecl*>(decl);
    EXPECT_NE(function, nullptr);
    EXPECT_EQ(function->get_name(), "main");
    EXPECT_EQ(function->num_params(), 0);
    
    auto body = function->get_body();
    EXPECT_NE(body, nullptr);

    auto block = dynamic_cast<const BlockStmt*>(body);
    EXPECT_NE(block, nullptr);
    EXPECT_EQ(block->size(), 1);

    auto stmt = block->get_stmts()[0];
    EXPECT_NE(stmt, nullptr);

    auto ret = dynamic_cast<RetStmt*>(stmt);
    EXPECT_NE(ret, nullptr);
    EXPECT_TRUE(ret->has_expr());

    auto val = ret->get_expr();
    EXPECT_NE(val, nullptr);

    auto integer = dynamic_cast<const IntegerLiteral*>(val);
    EXPECT_NE(integer, nullptr);
    EXPECT_EQ(integer->get_value(), 42);
}

} // namespace test

} // namespace stm
