#include "bytecode.hpp"

#include <cassert>

using namespace stm;

Operand Operand::get_register(ValueType type, vreg_t reg) {
    Operand operand;
    operand.kind = Kind::Register;
    operand.type = type;
    operand.reg = reg;

    return operand;
}

Operand Operand::get_memory(ValueType type, vreg_t reg, i32 offset) {
    Operand operand;
    operand.kind = Kind::Memory;
    operand.type = type;
    operand.memory.reg = reg;
    operand.memory.offset = offset;

    return operand;
}

Operand Operand::get_stack(ValueType type, i32 offset) {
    Operand operand;
    operand.kind = Kind::Stack;
    operand.type = type;
    operand.stack.offset = offset;

    return operand;
}

Operand Operand::get_imm(ValueType type, i64 imm) {
    Operand operand;
    operand.kind = Kind::Integer;
    operand.type = type;
    operand.imm = imm;

    return operand;
}   

Operand Operand::get_fp(ValueType type, f64 fp) {
    Operand operand;
    operand.kind = Kind::Float;
    operand.type = type;
    operand.fp = fp;

    return operand;
}

Operand Operand::get_string(const char* pString) {
    Operand operand;
    operand.kind = Kind::String;
    operand.type = ValueType::Pointer;
    operand.pString = pString;

    return operand;
}

Operand Operand::get_block(BasicBlock* pBlock) {
    Operand operand;
    operand.kind = Kind::Block;
    operand.type = ValueType::None;
    operand.pBlock = pBlock;

    return operand;
}

Operand Operand::get_function(Function* pFunction) {
    Operand operand;
    operand.kind = Kind::Function;
    operand.type = ValueType::None;
    operand.pFunction = pFunction;

    return operand;
}

Instruction::Instruction(
    u64 position,
    Opcode op, 
    const std::vector<Operand>& operands, 
    const InstDescriptor& desc, 
    const Metadata& meta,
    BasicBlock* pParent) 
        : position(position), op(op), operands(operands), desc(desc), 
          meta(meta), pParent(pParent) {
    if (pParent)
        pParent->append(this);
}; 

bool Instruction::is_terminator() const {
    return op == Opcode::Branch || op == Opcode::BranchIf || op == Opcode::Return;
}

BasicBlock::BasicBlock(Function* pParent) : pParent(pParent) {
    if (pParent)
        pParent->append(this);
}

BasicBlock::~BasicBlock() {

}

bool BasicBlock::terminates() const {
    // Start at the back of the block, since it is more likely to include 
    // terminators.
    for (const Instruction* curr = pBack; curr; curr = curr->prev())
        if (curr->is_terminator()) return true;

    return false;
}

u32 BasicBlock::terminators() const {
    u32 terminators = 0;
    for (const Instruction* curr = pFront; curr; curr = curr->next())
        if (curr->is_terminator()) terminators++;

    return terminators;
}

u32 BasicBlock::size() const {
    u32 size = 0;
    for (const Instruction* curr = pFront; curr; curr = curr->next()) size++;
    return size;
}

u32 BasicBlock::get_number() const {
    u32 number = 0;
    BasicBlock* prev = pPrev;
    while (prev) {
        number++;
        prev = prev->prev();
    }

    return number;
}

void BasicBlock::prepend(Instruction* pInst) {
    assert(pInst && "instruction cannot be null");

    if (pFront) {
        pFront->set_prev(pInst);
        pInst->set_next(pFront);
        pFront = pInst;
    } else {
        pFront = pInst;
        pBack = pInst;
    }
}

void BasicBlock::append(Instruction* pInst) {
    assert(pInst && "instruction cannot be null");

    if (pBack) {
        pBack->set_next(pInst);
        pInst->set_prev(pBack);
        pBack = pInst;
    } else {
        pFront = pInst;
        pBack = pInst;
    }
}

Function::Function(
        const std::string& name, 
        const std::vector<ValueType>& args, 
        ValueType ret)
    : name(name), args(args), ret(ret) {};

Function::~Function() {
    
}

u32 Function::num_blocks() const {
    u32 size = 0;
    for (const BasicBlock* curr = pFront; curr; curr = curr->next()) size++;
    return size;
}

void Function::prepend(BasicBlock* pBlock) {
    assert(pBlock && "basic block cannot be null");

    if (pFront) {
        pFront->set_prev(pBlock);
        pBlock->set_next(pFront);
        pFront = pBlock;
    } else {
        pFront = pBlock;
        pBack = pBlock;
    }
}

void Function::append(BasicBlock* pBlock) {
    assert(pBlock && "basic block cannot be null");

    if (pBack) {
        pBack->set_next(pBlock);
        pBlock->set_prev(pBack);
        pBack = pBlock;
    } else {
        pFront = pBlock;
        pBack = pBlock;
    }
}

Frame::~Frame() {
    
}
