#include "bytecode.hpp"
#include "logger.hpp"
#include "visitor.hpp"
#include "parser.hpp"
#include "translation_unit.hpp"

#include <gtest/gtest.h>

namespace stm {

namespace test {

class CodegenTest : public ::testing::Test {};

static inline void test_inst(
        Instruction* in,
        Opcode op, 
        Operand::Kind k) {
    EXPECT_EQ(in->opcode(), op);
    EXPECT_EQ(in->num_operands(), 1);
    EXPECT_EQ(in->operands()[0].kind, k);
}

static inline void test_inst(
        Instruction* in, 
        Opcode op, 
        Operand::Kind k1, 
        Operand::Kind k2) {
    EXPECT_EQ(in->opcode(), op);
    EXPECT_EQ(in->num_operands(), 2);
    EXPECT_EQ(in->operands()[0].kind, k1);
    EXPECT_EQ(in->operands()[1].kind, k2);
}

static constexpr Operand::Kind REGISTER = Operand::Kind::Register;
static constexpr Operand::Kind IMMEDIATE = Operand::Kind::Immediate;
static constexpr Operand::Kind MEMORY = Operand::Kind::Memory;

TEST_F(CodegenTest, cgn_numeric_increment_prefix) {
    InputFile file;
    file.pPath = "test";
    file.overwrite(R"(
        main :: () -> void {
            let x: i64;
            let y: i64 = ++x;
        }
    )");

    TranslationUnit unit { file };
    Parser parser { file };
    parser.parse(unit);

    Root& root = unit.get_root();
    root.validate();
    Options options;

    SymbolAnalysis syma { options, root };
    root.accept(syma);

    SemanticAnalysis sema { options, root };
    root.accept(sema);

    Codegen codegen { options, root };
    codegen.run(unit);

    Frame& frame = unit.get_frame();

    Function* fn1 = frame.get_function("main");
    EXPECT_NE(fn1, nullptr);

    BasicBlock* bb1 = fn1->front();
    EXPECT_NE(bb1, nullptr);
    EXPECT_EQ(bb1->size(), 6);

    // lea x, v1
    Instruction* in1 = bb1->front();
    test_inst(in1, Opcode::Lea, MEMORY, REGISTER);

    // mov [v1], v2
    Instruction* in2 = in1->next();
    test_inst(in2, Opcode::Move, MEMORY, REGISTER);

    // inc v2
    Instruction* in3 = in2->next();
    test_inst(in3, Opcode::Inc, REGISTER);

    // mov v2, (v1)
    Instruction* in4 = in3->next();
    test_inst(in4, Opcode::Move, REGISTER, MEMORY);

    // mov v2, (y)
    Instruction* in5 = in4->next();
    test_inst(in4, Opcode::Move, REGISTER, MEMORY);
}

TEST_F(CodegenTest, cgn_numeric_increment_postfix) {
    InputFile file;
    file.pPath = "test";
    file.overwrite(R"(
        main :: () -> void {
            let x: i64;
            let y: i64 = x++;
        }
    )");

    TranslationUnit unit { file };
    Parser parser { file };
    parser.parse(unit);

    Root& root = unit.get_root();
    root.validate();
    Options options;

    SymbolAnalysis syma { options, root };
    root.accept(syma);

    SemanticAnalysis sema { options, root };
    root.accept(sema);

    Codegen codegen { options, root };
    codegen.run(unit);

    Frame& frame = unit.get_frame();

    Function* fn1 = frame.get_function("main");
    EXPECT_NE(fn1, nullptr);

    BasicBlock* bb1 = fn1->front();
    EXPECT_NE(bb1, nullptr);
    EXPECT_EQ(bb1->size(), 7);

    // lea x, v1
    Instruction* in1 = bb1->front();
    EXPECT_EQ(in1->opcode(), Opcode::Lea);
    EXPECT_EQ(in1->operands()[0].kind, Operand::Kind::Memory);
    EXPECT_EQ(in1->operands()[1].kind, Operand::Kind::Register);

    // mov [v1], v2
    Instruction* in2 = in1->next();
    EXPECT_EQ(in2->opcode(), Opcode::Move);
    EXPECT_EQ(in2->operands()[0].kind, Operand::Kind::Memory);
    EXPECT_EQ(in2->operands()[1].kind, Operand::Kind::Register);

    // cpy v3 = v2
    Instruction* in3 = in2->next();
    EXPECT_EQ(in3->opcode(), Opcode::Copy);
    EXPECT_EQ(in3->operands()[0].kind, Operand::Kind::Register);
    EXPECT_EQ(in3->operands()[1].kind, Operand::Kind::Register);

    // inc v2
    Instruction* in4 = in3->next();
    EXPECT_EQ(in4->opcode(), Opcode::Inc);
    EXPECT_EQ(in4->operands()[0].kind, Operand::Kind::Register);

    // mov v2, (v1)
    Instruction* in5 = in4->next();
    EXPECT_EQ(in5->opcode(), Opcode::Move);
    EXPECT_EQ(in5->operands()[0].kind, Operand::Kind::Register);
    EXPECT_EQ(in5->operands()[1].kind, Operand::Kind::Memory);

    // mov v3, (y)
    Instruction* in6 = in5->next();
    EXPECT_EQ(in6->opcode(), Opcode::Move);
    EXPECT_EQ(in6->operands()[0].kind, Operand::Kind::Register);
    EXPECT_EQ(in6->operands()[1].kind, Operand::Kind::Memory);
}

TEST_F(CodegenTest, cgn_pointer_increment) {
    InputFile file;
    file.pPath = "test";
    file.overwrite(R"(
        main :: () -> void {
            let x: *i64;
            ++x;
        }
    )");

    TranslationUnit unit { file };
    Parser parser { file };
    parser.parse(unit);

    Root& root = unit.get_root();
    root.validate();
    Options options;

    SymbolAnalysis syma { options, root };
    root.accept(syma);

    SemanticAnalysis sema { options, root };
    root.accept(sema);

    Codegen codegen { options, root };
    codegen.run(unit);

    Frame& frame = unit.get_frame();

    Function* fn1 = frame.get_function("main");
    EXPECT_NE(fn1, nullptr);

    BasicBlock* bb1 = fn1->front();
    EXPECT_NE(bb1, nullptr);
    EXPECT_EQ(bb1->size(), 6);

    // lea x, v1
    Instruction* in1 = bb1->front();
    EXPECT_EQ(in1->opcode(), Opcode::Lea);
    EXPECT_EQ(in1->operands()[0].kind, Operand::Kind::Memory);
    EXPECT_EQ(in1->operands()[1].kind, Operand::Kind::Register);

    // mov (v1), v2
    Instruction* in2 = in1->next();
    EXPECT_EQ(in2->opcode(), Opcode::Move);
    EXPECT_EQ(in2->operands()[0].kind, Operand::Kind::Memory);
    EXPECT_EQ(in2->operands()[1].kind, Operand::Kind::Register);
    
    // const v3 = 8
    Instruction* in3 = in2->next();
    EXPECT_EQ(in3->opcode(), Opcode::Constant);
    EXPECT_EQ(in3->operands()[0].kind, Operand::Kind::Immediate);
    EXPECT_EQ(in3->operands()[1].kind, Operand::Kind::Register);

    // add v3, v2
    Instruction* in4 = in3->next();
    EXPECT_EQ(in4->opcode(), Opcode::Add);
    EXPECT_EQ(in4->operands()[0].kind, Operand::Kind::Register);
    EXPECT_EQ(in4->operands()[1].kind, Operand::Kind::Register);

    // mov v2, (v1)
    Instruction* in5 = in4->next();
    EXPECT_EQ(in5->opcode(), Opcode::Move);
    EXPECT_EQ(in5->operands()[0].kind, Operand::Kind::Register);
    EXPECT_EQ(in5->operands()[1].kind, Operand::Kind::Memory);
}

TEST_F(CodegenTest, cgn_member_assignment) {
    InputFile file;
    file.pPath = "test";
    file.overwrite(R"(
        box :: {
            a: i64,
            b: i64,
            c: i64
        }

        main :: () -> void {
            let x: box;
            x.b = 1;
        }
    )");

    TranslationUnit unit { file };
    Parser parser { file };
    parser.parse(unit);

    Root& root = unit.get_root();
    root.validate();
    Options options;

    SymbolAnalysis syma { options, root };
    root.accept(syma);

    SemanticAnalysis sema { options, root };
    root.accept(sema);

    Codegen codegen { options, root };
    codegen.run(unit);

    Frame& frame = unit.get_frame();

    Function* fn1 = frame.get_function("main");
    EXPECT_NE(fn1, nullptr);

    BasicBlock* bb1 = fn1->front();
    EXPECT_NE(bb1, nullptr);
    EXPECT_EQ(bb1->size(), 4);

    // lea x, v1
    Instruction* in1 = bb1->front();
    EXPECT_EQ(in1->opcode(), Opcode::Lea);
    EXPECT_EQ(in1->num_operands(), 2);
    EXPECT_EQ(in1->operands()[0].kind, Operand::Kind::Memory);
    EXPECT_EQ(in1->operands()[1].kind, Operand::Kind::Register);

    // const v2 = 1
    Instruction* in2 = in1->next();
    EXPECT_EQ(in2->opcode(), Opcode::Constant);
    EXPECT_EQ(in2->num_operands(), 2);
    EXPECT_EQ(in2->operands()[0].kind, Operand::Kind::Immediate);
    EXPECT_EQ(in2->operands()[1].kind, Operand::Kind::Register);

    // mov v2, [v1+8]
    Instruction* in3 = in2->next();
    EXPECT_EQ(in3->opcode(), Opcode::Move);
    EXPECT_EQ(in3->num_operands(), 2);
    EXPECT_EQ(in3->operands()[0].kind, Operand::Kind::Register);
    EXPECT_EQ(in3->operands()[1].kind, Operand::Kind::Memory);

    // ret
    Instruction* in4 = in3->next();
    EXPECT_EQ(in4->opcode(), Opcode::Return);
}

TEST_F(CodegenTest, cgn_if_explicit_int_cond) {
    InputFile file;
    file.pPath = "test";
    file.overwrite(R"(
        main :: () -> void {
            let x: i64 = 4;
            
            if x == 3 {
                ret;
            }

            ret;
        }
    )");

    TranslationUnit unit { file };
    Parser parser { file };
    parser.parse(unit);

    Root& root = unit.get_root();
    root.validate();
    Options options;

    SymbolAnalysis syma { options, root };
    root.accept(syma);

    SemanticAnalysis sema { options, root };
    root.accept(sema);

    Codegen codegen { options, root };
    codegen.run(unit);

    Frame& frame = unit.get_frame();

    Function* fn1 = frame.get_function("main");
    EXPECT_NE(fn1, nullptr);

    BasicBlock* bb1 = fn1->front();
    EXPECT_NE(bb1, nullptr);
    EXPECT_EQ(bb1->size(), 7);

    // constant v = 4
    Instruction* in1 = bb1->front();
    EXPECT_EQ(in1->opcode(), Opcode::Constant);
    EXPECT_EQ(in1->num_operands(), 2);
    EXPECT_EQ(in1->operands()[0].kind, Operand::Kind::Immediate);
    EXPECT_EQ(in1->operands()[1].kind, Operand::Kind::Register);

    // move v, stack[x]
    Instruction* in2 = in1->next();
    EXPECT_EQ(in2->opcode(), Opcode::Move);
    EXPECT_EQ(in2->num_operands(), 2);
    EXPECT_EQ(in2->operands()[0].kind, Operand::Kind::Register);
    EXPECT_EQ(in2->operands()[1].kind, Operand::Kind::Memory);

    // move v = x
    Instruction* in3 = in2->next();
    EXPECT_EQ(in3->opcode(), Opcode::Move);
    EXPECT_EQ(in3->num_operands(), 2);
    EXPECT_EQ(in3->operands()[0].kind, Operand::Kind::Memory);
    EXPECT_EQ(in3->operands()[1].kind, Operand::Kind::Register);

    // constant v = 3
    Instruction* in4 = in3->next();
    EXPECT_EQ(in4->opcode(), Opcode::Constant);
    EXPECT_EQ(in4->num_operands(), 2);
    EXPECT_EQ(in4->operands()[0].kind, Operand::Kind::Immediate);
    EXPECT_EQ(in4->operands()[1].kind, Operand::Kind::Register);

    // cmpeq x, 3
    Instruction* in5 = in4->next();
    EXPECT_EQ(in5->opcode(), Opcode::Cmpeq);
    EXPECT_EQ(in5->num_operands(), 2);
    EXPECT_EQ(in5->operands()[0].kind, Operand::Kind::Register);
    EXPECT_EQ(in5->operands()[1].kind, Operand::Kind::Register);

    // brf bb3
    Instruction* in6 = in5->next();
    EXPECT_EQ(in6->opcode(), Opcode::BranchFalse);
    EXPECT_EQ(in6->num_operands(), 1);
    EXPECT_EQ(in6->operands()[0].kind, Operand::Kind::Block);

    // jmp bb2
    Instruction* in7 = in6->next();
    EXPECT_EQ(in7->opcode(), Opcode::Jump);
    EXPECT_EQ(in7->num_operands(), 1);
    EXPECT_EQ(in7->operands()[0].kind, Operand::Kind::Block);
}

TEST_F(CodegenTest, cgn_if_explicit_fp_cond) {
    InputFile file;
    file.pPath = "test";
    file.overwrite(R"(
        main :: () -> void {
            let x: f64 = 4.0;
            
            if x == 3.0 {
                ret;
            }

            ret;
        }
    )");

    TranslationUnit unit { file };
    Parser parser { file };
    parser.parse(unit);

    Root& root = unit.get_root();
    root.validate();
    Options options;

    SymbolAnalysis syma { options, root };
    root.accept(syma);

    SemanticAnalysis sema { options, root };
    root.accept(sema);

    Codegen codegen { options, root };
    codegen.run(unit);

    Frame& frame = unit.get_frame();

    Function* fn1 = frame.get_function("main");
    EXPECT_NE(fn1, nullptr);

    BasicBlock* bb1 = fn1->front();
    EXPECT_NE(bb1, nullptr);
    EXPECT_EQ(bb1->size(), 7);

    // constant v = 4
    Instruction* in1 = bb1->front();
    EXPECT_EQ(in1->opcode(), Opcode::Constant);
    EXPECT_EQ(in1->num_operands(), 2);
    EXPECT_EQ(in1->operands()[0].kind, Operand::Kind::Immediate);
    EXPECT_EQ(in1->operands()[1].kind, Operand::Kind::Register);

    // move v, stack[x]
    Instruction* in2 = in1->next();
    EXPECT_EQ(in2->opcode(), Opcode::Move);
    EXPECT_EQ(in2->num_operands(), 2);
    EXPECT_EQ(in2->operands()[0].kind, Operand::Kind::Register);
    EXPECT_EQ(in2->operands()[1].kind, Operand::Kind::Memory);

    // move v = x
    Instruction* in3 = in2->next();
    EXPECT_EQ(in3->opcode(), Opcode::Move);
    EXPECT_EQ(in3->num_operands(), 2);
    EXPECT_EQ(in3->operands()[0].kind, Operand::Kind::Memory);
    EXPECT_EQ(in3->operands()[1].kind, Operand::Kind::Register);

    // constant v = 3
    Instruction* in4 = in3->next();
    EXPECT_EQ(in4->opcode(), Opcode::Constant);
    EXPECT_EQ(in4->num_operands(), 2);
    EXPECT_EQ(in4->operands()[0].kind, Operand::Kind::Immediate);
    EXPECT_EQ(in4->operands()[1].kind, Operand::Kind::Register);

    // cmp x, 3
    Instruction* in5 = in4->next();
    EXPECT_EQ(in5->opcode(), Opcode::Cmpoeq);
    EXPECT_EQ(in5->num_operands(), 2);
    EXPECT_EQ(in5->operands()[0].kind, Operand::Kind::Register);
    EXPECT_EQ(in5->operands()[1].kind, Operand::Kind::Register);

    // brf bb3
    Instruction* in6 = in5->next();
    EXPECT_EQ(in6->opcode(), Opcode::BranchFalse);
    EXPECT_EQ(in6->num_operands(), 1);
    EXPECT_EQ(in6->operands()[0].kind, Operand::Kind::Block);

    // jmp bb2
    Instruction* in7 = in6->next();
    EXPECT_EQ(in7->opcode(), Opcode::Jump);
    EXPECT_EQ(in7->num_operands(), 1);
    EXPECT_EQ(in7->operands()[0].kind, Operand::Kind::Block);
}

} // namespace test

} // namespace stm
