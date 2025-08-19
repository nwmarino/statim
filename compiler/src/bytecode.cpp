#include "bytecode.hpp"

#include <cassert>

using namespace stm;

Operand::Operand(Register reg)
    : kind(Kind::Register), reg(reg) {};

Operand::Operand(Immediate imm)
    : kind(Kind::Immediate), imm(imm) {};

Operand::Operand(MemoryRef mem)
    : kind(Kind::MemoryRef), mem(mem) {};

Operand::Operand(StackRef stack)
    : kind(Kind::StackRef), stack(stack) {};

Operand::Operand(BlockRef block)
    : kind(Kind::BlockRef), block(block) {};

Operand::Operand(FunctionRef function) 
    : kind(Kind::FunctionRef), function(function) {};

u32 Instruction::s_position = 0;

Instruction::Instruction(
        Opcode op, 
        const std::vector<Operand>& operands, 
        const Metadata& meta,
        const InstructionDesc& desc, 
        BasicBlock* pParent) 
    : pos(s_position++), op(op), operands(operands), meta(meta), desc(desc), 
      pParent(pParent) {
    if (pParent)
        pParent->append(this);
}; 

bool Instruction::is_terminator() const {
    return op == Opcode::Branch || op == Opcode::Jump || 
        op == Opcode::Return;
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

StackSlot::StackSlot(
        const std::string& name, 
        u32 offset, 
        Function* pParent)
    : name(name), offset(offset), pParent(pParent) {
    if (pParent)
        pParent->add_slot(this);
};

Function::Function(
        const std::string& name, 
        const std::vector<ValueType>& args, 
        ValueType ret)
    : name(name), args(args), ret(ret) {};

Function::~Function() {
    
}

u32 Function::get_stack_size() const {
    if (stack.empty())
        return 0;

    return stack.end()->second->get_offset();
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
