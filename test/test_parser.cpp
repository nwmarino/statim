#include "tree/decl.hpp"
#include "tree/expr.hpp"
#include "tree/parser.hpp"
#include "tree/root.hpp"
#include "tree/rune.hpp"
#include "tree/stmt.hpp"
#include "types/translation_unit.hpp"

#include <gtest/gtest.h>

namespace stm {

namespace test {

class ParserTest : public ::testing::Test {};

TEST_F(ParserTest, parse_function_basic) {
    InputFile file;
    file.path = "test";
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

TEST_F(ParserTest, parse_function_params) {
    InputFile file;
    file.path = "test";
    file.overwrite("main :: (a: i64, b: u32) -> void {}");

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
    EXPECT_EQ(function->num_params(), 2);

    auto param1 = function->get_param(0);
    EXPECT_NE(param1, nullptr);
    EXPECT_EQ(param1->get_name(), "a");
    EXPECT_EQ(param1->get_type()->to_string(), "i64");

    auto param2 = function->get_param(1);
    EXPECT_NE(param2, nullptr);
    EXPECT_EQ(param2->get_name(), "b");
    EXPECT_EQ(param2->get_type()->to_string(), "u32");
}

TEST_F(ParserTest, parse_variable_local) {
    InputFile file;
    file.path = "test";
    file.overwrite("main :: () -> void { let x: u32; }");

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

    auto body = function->get_body();
    EXPECT_NE(body, nullptr);

    auto block = dynamic_cast<const BlockStmt*>(body);
    EXPECT_NE(block, nullptr);
    EXPECT_EQ(block->size(), 1);

    auto stmt = block->get_stmts()[0];
    EXPECT_NE(stmt, nullptr);

    auto decl_stmt = dynamic_cast<const DeclStmt*>(stmt);
    EXPECT_NE(decl_stmt, nullptr);

    auto decl_stmt_decl = decl_stmt->get_decl();
    EXPECT_NE(decl_stmt_decl, nullptr);
    EXPECT_EQ(decl_stmt_decl->get_name(), "x");

    auto var = dynamic_cast<const VariableDecl*>(decl_stmt_decl);
    EXPECT_NE(var, nullptr);
    EXPECT_EQ(var->get_type()->to_string(), "u32");
    EXPECT_FALSE(var->has_init());
}

TEST_F(ParserTest, parse_variable_local_with_init) {
    InputFile file;
    file.path = "test";
    file.overwrite("main :: () -> void { let x: u32 = 1; }");

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

    auto body = function->get_body();
    EXPECT_NE(body, nullptr);

    auto block = dynamic_cast<const BlockStmt*>(body);
    EXPECT_NE(block, nullptr);
    EXPECT_EQ(block->size(), 1);

    auto stmt = block->get_stmts()[0];
    EXPECT_NE(stmt, nullptr);

    auto decl_stmt = dynamic_cast<const DeclStmt*>(stmt);
    EXPECT_NE(decl_stmt, nullptr);

    auto decl_stmt_decl = decl_stmt->get_decl();
    EXPECT_NE(decl_stmt_decl, nullptr);
    EXPECT_EQ(decl_stmt_decl->get_name(), "x");

    auto var = dynamic_cast<const VariableDecl*>(decl_stmt_decl);
    EXPECT_NE(var, nullptr);
    EXPECT_EQ(var->get_type()->to_string(), "u32");
    EXPECT_TRUE(var->has_init());

    auto init = var->get_init();
    EXPECT_NE(init, nullptr);

    auto integer = dynamic_cast<const IntegerLiteral*>(init);
    EXPECT_NE(integer, nullptr);
    EXPECT_EQ(integer->get_value(), 1);
}

TEST_F(ParserTest, parse_struct) {
    InputFile file;
    file.path = "test";
    file.overwrite("box :: { length: u32, width: i32, height: u64 }");

    TranslationUnit unit { file };
    Parser parser { file };
    parser.parse(unit);
    
    Root& root = unit.get_root();
    EXPECT_EQ(root.num_decls(), 1);

    auto decl = root.get_decls()[0];
    EXPECT_NE(decl, nullptr);
    
    auto structure = dynamic_cast<const StructDecl*>(decl);
    EXPECT_NE(structure, nullptr);
    EXPECT_EQ(structure->get_name(), "box");
    EXPECT_EQ(structure->num_fields(), 3);

    auto fields = structure->get_fields();

    auto field1 = fields[0];
    EXPECT_EQ(field1->get_name(), "length");
    EXPECT_EQ(field1->get_type()->to_string(), "u32");
    EXPECT_EQ(field1->get_index(), 0);
    EXPECT_EQ(field1->get_parent(), structure);

    auto field2 = fields[1];
    EXPECT_EQ(field2->get_name(), "width");
    EXPECT_EQ(field2->get_type()->to_string(), "i32");
    EXPECT_EQ(field2->get_index(), 1);
    EXPECT_EQ(field2->get_parent(), structure);

    auto field3 = fields[2];
    EXPECT_EQ(field3->get_name(), "height");
    EXPECT_EQ(field3->get_type()->to_string(), "u64");
    EXPECT_EQ(field3->get_index(), 2);
    EXPECT_EQ(field3->get_parent(), structure);
}

TEST_F(ParserTest, parse_enum) {
    InputFile file;
    file.path = "test";
    file.overwrite("colors :: i16 { RED, BLUE = 54, GREEN, Yellow = 1, ORANGE }");

    TranslationUnit unit { file };
    Parser parser { file };
    parser.parse(unit);
    
    Root& root = unit.get_root();
    EXPECT_EQ(root.num_decls(), 1);

    auto decl = root.get_decls()[0];
    EXPECT_NE(decl, nullptr);

    auto enumeration = dynamic_cast<const EnumDecl*>(decl);
    EXPECT_NE(enumeration, nullptr);
    EXPECT_EQ(enumeration->get_name(), "colors");
    EXPECT_EQ(enumeration->get_type()->get_underlying()->to_string(), "i16");
    EXPECT_EQ(enumeration->num_values(), 5);

    auto values = enumeration->get_values();

    auto value1 = values[0];
    EXPECT_EQ(value1->get_name(), "RED");
    EXPECT_EQ(value1->get_value(), 0);

    auto value2 = values[1];
    EXPECT_EQ(value2->get_name(), "BLUE");
    EXPECT_EQ(value2->get_value(), 54);

    auto value3 = values[2];
    EXPECT_EQ(value3->get_name(), "GREEN");
    EXPECT_EQ(value3->get_value(), 55);

    auto value4 = values[3];
    EXPECT_EQ(value4->get_name(), "Yellow");
    EXPECT_EQ(value4->get_value(), 1);

    auto value5 = values[4];
    EXPECT_EQ(value5->get_name(), "ORANGE");
    EXPECT_EQ(value5->get_value(), 2);
}

TEST_F(ParserTest, parse_break_stmt) {
    InputFile file;
    file.path = "test";
    file.overwrite("main :: () -> void { break; }");

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

    auto body = function->get_body();
    EXPECT_NE(body, nullptr);

    auto block = dynamic_cast<const BlockStmt*>(body);
    EXPECT_NE(block, nullptr);
    EXPECT_EQ(block->size(), 1);

    auto stmt = block->get_stmts()[0];
    EXPECT_NE(stmt, nullptr);

    auto break_stmt = dynamic_cast<const BreakStmt*>(stmt);
    EXPECT_NE(break_stmt, nullptr);
}

TEST_F(ParserTest, parse_continue_stmt) {
    InputFile file;
    file.path = "test";
    file.overwrite("main :: () -> void { continue; }");

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

    auto body = function->get_body();
    EXPECT_NE(body, nullptr);

    auto block = dynamic_cast<const BlockStmt*>(body);
    EXPECT_NE(block, nullptr);
    EXPECT_EQ(block->size(), 1);

    auto stmt = block->get_stmts()[0];
    EXPECT_NE(stmt, nullptr);

    auto continue_stmt = dynamic_cast<const ContinueStmt*>(stmt);
    EXPECT_NE(continue_stmt, nullptr);
}

TEST_F(ParserTest, parse_if_stmt) {
    InputFile file;
    file.path = "test";
    file.overwrite("main :: () -> void { if 365 {} }");

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

    auto body = function->get_body();
    EXPECT_NE(body, nullptr);

    auto block = dynamic_cast<const BlockStmt*>(body);
    EXPECT_NE(block, nullptr);
    EXPECT_EQ(block->size(), 1);

    auto stmt = block->get_stmts()[0];
    EXPECT_NE(stmt, nullptr);

    auto if_stmt = dynamic_cast<const IfStmt*>(stmt);
    EXPECT_NE(if_stmt, nullptr);
    EXPECT_FALSE(if_stmt->has_else());

    auto if_cond = if_stmt->get_cond();
    EXPECT_NE(if_cond, nullptr);

    auto integer = dynamic_cast<const IntegerLiteral*>(if_cond);
    EXPECT_NE(integer, nullptr);
    EXPECT_EQ(integer->get_value(), 365);

    auto then_stmt = if_stmt->get_then();
    EXPECT_NE(then_stmt, nullptr);

    auto block2 = dynamic_cast<const BlockStmt*>(then_stmt);
    EXPECT_NE(block2, nullptr);
    EXPECT_EQ(block2->size(), 0);
}

TEST_F(ParserTest, parse_if_stmt_with_else) {
    InputFile file;
    file.path = "test";
    file.overwrite("main :: () -> void { if 41 {} else {} }");

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

    auto body = function->get_body();
    EXPECT_NE(body, nullptr);

    auto block = dynamic_cast<const BlockStmt*>(body);
    EXPECT_NE(block, nullptr);
    EXPECT_EQ(block->size(), 1);

    auto stmt = block->get_stmts()[0];
    EXPECT_NE(stmt, nullptr);

    auto if_stmt = dynamic_cast<const IfStmt*>(stmt);
    EXPECT_NE(if_stmt, nullptr);
    EXPECT_TRUE(if_stmt->has_else());

    auto if_cond = if_stmt->get_cond();
    EXPECT_NE(if_cond, nullptr);

    auto integer = dynamic_cast<const IntegerLiteral*>(if_cond);
    EXPECT_NE(integer, nullptr);
    EXPECT_EQ(integer->get_value(), 41);

    auto then_stmt = if_stmt->get_then();
    EXPECT_NE(then_stmt, nullptr);

    auto block2 = dynamic_cast<const BlockStmt*>(then_stmt);
    EXPECT_NE(block2, nullptr);
    EXPECT_EQ(block2->size(), 0);

    auto else_stmt = if_stmt->get_else();
    EXPECT_NE(else_stmt, nullptr);

    auto block3 = dynamic_cast<const BlockStmt*>(else_stmt);
    EXPECT_NE(block3, nullptr);
    EXPECT_EQ(block3->size(), 0);
}

TEST_F(ParserTest, parse_if_stmt_with_else_if_else) {
    InputFile file;
    file.path = "test";
    file.overwrite("main :: () -> void { if 0 {} else if 42 {} else {} }");

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

    auto body = function->get_body();
    EXPECT_NE(body, nullptr);

    auto block = dynamic_cast<const BlockStmt*>(body);
    EXPECT_NE(block, nullptr);
    EXPECT_EQ(block->size(), 1);

    auto stmt = block->get_stmts()[0];
    EXPECT_NE(stmt, nullptr);

    auto if_stmt = dynamic_cast<const IfStmt*>(stmt);
    EXPECT_NE(if_stmt, nullptr);
    EXPECT_TRUE(if_stmt->has_else());

    auto if_cond = if_stmt->get_cond();
    EXPECT_NE(if_cond, nullptr);

    auto integer = dynamic_cast<const IntegerLiteral*>(if_cond);
    EXPECT_NE(integer, nullptr);
    EXPECT_EQ(integer->get_value(), 0);

    auto then_stmt = if_stmt->get_then();
    EXPECT_NE(then_stmt, nullptr);

    auto block2 = dynamic_cast<const BlockStmt*>(then_stmt);
    EXPECT_NE(block2, nullptr);
    EXPECT_EQ(block2->size(), 0);

    auto else_stmt = if_stmt->get_else();
    EXPECT_NE(else_stmt, nullptr);

    auto if_stmt2 = dynamic_cast<const IfStmt*>(else_stmt);
    EXPECT_NE(if_stmt2, nullptr);
    EXPECT_TRUE(if_stmt2->has_else());
    
    auto if_cond2 = if_stmt2->get_cond();
    EXPECT_NE(if_cond2, nullptr);

    auto integer2 = dynamic_cast<const IntegerLiteral*>(if_cond2);
    EXPECT_NE(integer2, nullptr);
    EXPECT_EQ(integer2->get_value(), 42);

    auto else_stmt2 = if_stmt2->get_else();
    EXPECT_NE(else_stmt2, nullptr);

    auto block3 = dynamic_cast<const BlockStmt*>(else_stmt2);
    EXPECT_NE(block3, nullptr);
    EXPECT_EQ(block3->size(), 0);
}

TEST_F(ParserTest, parse_while_stmt) {
    InputFile file;
    file.path = "test";
    file.overwrite("main :: () -> void { while 77 {}; }");

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

    auto while_stmt = dynamic_cast<const WhileStmt*>(stmt);
    EXPECT_NE(while_stmt, nullptr);
    EXPECT_TRUE(while_stmt->has_body());

    auto while_cond = while_stmt->get_cond();
    EXPECT_NE(while_cond, nullptr);

    auto integer = dynamic_cast<const IntegerLiteral*>(while_cond);
    EXPECT_NE(integer, nullptr);
    EXPECT_EQ(integer->get_value(), 77);

    auto while_body = while_stmt->get_body();
    EXPECT_NE(while_body, nullptr);

    auto block2 = dynamic_cast<const BlockStmt*>(while_body);
    EXPECT_NE(block2, nullptr);
    EXPECT_EQ(block2->size(), 0);
}

TEST_F(ParserTest, parse_return_basic) {
    InputFile file;
    file.path = "test";
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

TEST_F(ParserTest, parse_bool_literal) {
    InputFile file;
    file.path = "test";
    file.overwrite("main :: () -> void { true; }");

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

    auto boolean = dynamic_cast<const BoolLiteral*>(stmt);
    EXPECT_NE(boolean, nullptr);
    EXPECT_EQ(boolean->get_value(), true);
}

TEST_F(ParserTest, parse_float_literal) {
    InputFile file;
    file.path = "test";
    file.overwrite("main :: () -> void { 3.14; }");

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

    auto fp = dynamic_cast<const FloatLiteral*>(stmt);
    EXPECT_NE(fp, nullptr);
    EXPECT_EQ(fp->get_value(), 3.14);
}

TEST_F(ParserTest, parse_char_literal) {
    InputFile file;
    file.path = "test";
    file.overwrite("main :: () -> void { 'z'; }");

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

    auto character = dynamic_cast<const CharLiteral*>(stmt);
    EXPECT_NE(character, nullptr);
    EXPECT_EQ(character->get_value(), 'z');
}

TEST_F(ParserTest, parse_string_literal) {
    InputFile file;
    file.path = "test";
    file.overwrite("main :: () -> void { \"abc\"; }");

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

    auto string = dynamic_cast<const StringLiteral*>(stmt);
    EXPECT_NE(string, nullptr);
    EXPECT_EQ(string->get_value(), "abc");
}

TEST_F(ParserTest, parse_null_literal) {
    InputFile file;
    file.path = "test";
    file.overwrite("main :: () -> void { null; }");

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

    auto null = dynamic_cast<const NullLiteral*>(stmt);
    EXPECT_NE(null, nullptr);
}

TEST_F(ParserTest, parse_binary_expr_basic) {
    InputFile file;
    file.path = "test";
    file.overwrite("main :: () -> void { 1 + 3.14; }");

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

    auto binary = dynamic_cast<const BinaryExpr*>(stmt);
    EXPECT_NE(binary, nullptr);
    EXPECT_EQ(binary->get_operator(), BinaryExpr::Operator::Add);

    auto lhs = binary->get_lhs();
    EXPECT_NE(lhs, nullptr);

    auto integer = dynamic_cast<const IntegerLiteral*>(lhs);
    EXPECT_NE(integer, nullptr);
    EXPECT_EQ(integer->get_value(), 1);

    auto rhs = binary->get_rhs();
    EXPECT_NE(rhs, nullptr);

    auto fp = dynamic_cast<const FloatLiteral*>(rhs);
    EXPECT_NE(fp, nullptr);
    EXPECT_EQ(fp->get_value(), 3.14);
}

TEST_F(ParserTest, parse_binary_expr_extended) {

}

TEST_F(ParserTest, parse_binary_expr_complex) {

}

TEST_F(ParserTest, parse_unary_expr_prefix_basic) {
    InputFile file;
    file.path = "test";
    file.overwrite("main :: () -> void { *5; }");

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

    auto unary = dynamic_cast<const UnaryExpr*>(stmt);
    EXPECT_NE(unary, nullptr);
    EXPECT_EQ(unary->get_operator(), UnaryExpr::Operator::Dereference);
    EXPECT_TRUE(unary->is_prefix());
}

TEST_F(ParserTest, parse_unary_expr_postfix_basic) {
    InputFile file;
    file.path = "test";
    file.overwrite("main :: () -> void { 5++; }");

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

    auto unary = dynamic_cast<const UnaryExpr*>(stmt);
    EXPECT_NE(unary, nullptr);
    EXPECT_EQ(unary->get_operator(), UnaryExpr::Operator::Increment);
    EXPECT_TRUE(unary->is_postfix());
}

TEST_F(ParserTest, parse_unary_expr_complex) {
    InputFile file;
    file.path = "test";
    file.overwrite("main :: () -> void { *5++; }");

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

    auto unary = dynamic_cast<const UnaryExpr*>(stmt);
    EXPECT_NE(unary, nullptr);
    EXPECT_EQ(unary->get_operator(), UnaryExpr::Operator::Dereference);
    EXPECT_TRUE(unary->is_prefix());

    auto unary_expr = unary->get_expr();
    EXPECT_NE(unary_expr, nullptr);

    auto unary2 = dynamic_cast<const UnaryExpr*>(unary_expr);
    EXPECT_NE(unary2, nullptr);
    EXPECT_EQ(unary2->get_operator(), UnaryExpr::Operator::Increment);
    EXPECT_TRUE(unary2->is_postfix());
}

TEST_F(ParserTest, parse_cast_expr) {
    InputFile file;
    file.path = "test";
    file.overwrite("main :: () -> void { cast<u32>(5); }");

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

    auto cast = dynamic_cast<const CastExpr*>(stmt);
    EXPECT_NE(cast, nullptr);
    EXPECT_EQ(cast->get_type()->to_string(), "u32");

    auto cast_expr = cast->get_expr();
    EXPECT_NE(cast_expr, nullptr);
    
    auto integer = dynamic_cast<const IntegerLiteral*>(cast_expr);
    EXPECT_NE(integer, nullptr);
    EXPECT_EQ(integer->get_value(), 5);
}

TEST_F(ParserTest, parse_paren_expr) {
    InputFile file;
    file.path = "test";
    file.overwrite("main :: () -> void { (5); }");

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

    auto paren = dynamic_cast<const ParenExpr*>(stmt);
    EXPECT_NE(paren, nullptr);

    auto paren_expr = paren->get_expr();
    EXPECT_NE(paren_expr, nullptr);

    auto integer = dynamic_cast<const IntegerLiteral*>(paren_expr);
    EXPECT_NE(integer, nullptr);
    EXPECT_EQ(integer->get_value(), 5);
}

TEST_F(ParserTest, parse_sizeof_expr) {
    InputFile file;
    file.path = "test";
    file.overwrite("main :: () -> void { sizeof(u32); }");

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

    auto size_of = dynamic_cast<const SizeofExpr*>(stmt);
    EXPECT_NE(size_of, nullptr);
    EXPECT_EQ(size_of->get_target()->to_string(), "u32");
}

TEST_F(ParserTest, parse_subscript_expr) {
    InputFile file;
    file.path = "test";
    file.overwrite("main :: () -> void { x[42]; }");

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

    auto subscript = dynamic_cast<const SubscriptExpr*>(stmt);
    EXPECT_NE(subscript, nullptr);
    
    auto base = subscript->get_base();
    EXPECT_NE(base, nullptr);

    auto reference = dynamic_cast<const ReferenceExpr*>(base);
    EXPECT_NE(reference, nullptr);
    EXPECT_EQ(reference->get_name(), "x");

    auto index = subscript->get_index();
    EXPECT_NE(index, nullptr);

    auto integer = dynamic_cast<const IntegerLiteral*>(index);
    EXPECT_NE(integer, nullptr);
    EXPECT_EQ(integer->get_value(), 42);
}

TEST_F(ParserTest, parse_member_expr) {
    InputFile file;
    file.path = "test";
    file.overwrite("main :: () -> void { x.a; }");

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

    auto member = dynamic_cast<const MemberExpr*>(stmt);
    EXPECT_NE(member, nullptr);
    EXPECT_EQ(member->get_name(), "a");

    auto base = member->get_base();
    EXPECT_NE(base, nullptr);

    auto ref = dynamic_cast<const ReferenceExpr*>(base);
    EXPECT_NE(ref, nullptr);
    EXPECT_EQ(ref->get_name(), "x");
}

TEST_F(ParserTest, parse_call_expr) {
    InputFile file;
    file.path = "test";
    file.overwrite("main :: () -> void { foo(); }");

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

    auto call = dynamic_cast<const CallExpr*>(stmt);
    EXPECT_NE(call, nullptr);
    EXPECT_EQ(call->get_name(), "foo");
    EXPECT_EQ(call->num_args(), 0);
}

TEST_F(ParserTest, parse_call_expr_with_args) {
    InputFile file;
    file.path = "test";
    file.overwrite("main :: () -> void { foo(1, y); }");

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

    auto call = dynamic_cast<const CallExpr*>(stmt);
    EXPECT_NE(call, nullptr);
    EXPECT_EQ(call->get_name(), "foo");
    EXPECT_EQ(call->num_args(), 2);

    auto arg1 = call->get_args()[0];
    EXPECT_NE(arg1, nullptr);

    auto integer = dynamic_cast<const IntegerLiteral*>(arg1);
    EXPECT_NE(integer, nullptr);
    EXPECT_EQ(integer->get_value(), 1);

    auto arg2 = call->get_args()[1];
    EXPECT_NE(arg2, nullptr);

    auto ref = dynamic_cast<const ReferenceExpr*>(arg2);
    EXPECT_NE(ref, nullptr);
    EXPECT_EQ(ref->get_name(), "y");
}

} // namespace test

} // namespace stm
