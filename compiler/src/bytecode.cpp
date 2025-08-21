#include "bytecode.hpp"

#include <cassert>

using namespace stm;

Operand::Operand(Register reg) : kind(Kind::Register), reg(reg) {};

Operand::Operand(Immediate imm) : kind(Kind::Immediate), imm(imm) {};

Operand::Operand(MemoryRef mem) : kind(Kind::Memory), mem(mem) {};

Operand::Operand(ArgumentRef arg) : kind(Kind::Argument), arg(arg) {};

Operand::Operand(ReturnRef ret) : kind(Kind::Return), ret(ret) {};

Operand::Operand(BlockRef block) : kind(Kind::Block), block(block) {};

Operand::Operand(FunctionRef function) 
    : kind(Kind::Function), function(function) {};

Instruction::Instruction(
        u32 position,
        Opcode op, 
        const std::vector<Operand>& operands, 
        const Metadata& meta,
        BasicBlock* insert,
        Size size,
        const std::string& comment)
    : m_position(position), m_op(op), m_operands(operands), m_meta(meta), 
      m_size(size), m_comment(comment), m_parent(insert) {

    if (m_parent) 
        m_parent->append(this);
}

void Instruction::create(
        BasicBlock* block,
        Opcode op, 
        const std::vector<Operand>& operands, 
        const Metadata& meta,
        Size size,
        const std::string& comment) {
    static u32 instr_pos = 0;
    new Instruction(
        instr_pos++,
        op,
        operands,
        meta,
        block,
        size,
        comment);
}

bool Instruction::is_terminator() const {
    switch (m_op) {
    case Opcode::Jump:
    case Opcode::BranchTrue:
    case Opcode::BranchFalse:
    case Opcode::Return:
        return true;
    default:
        return false;
    }
}

bool Instruction::is_comparison() const {
    switch (m_op) {
    case Opcode::Cmpeq:
    case Opcode::Cmpne:
    case Opcode::Cmpoeq:
    case Opcode::Cmpone:
    case Opcode::Cmpuneq:
    case Opcode::Cmpunne:
    case Opcode::Cmpslt:
    case Opcode::Cmpsle:
    case Opcode::Cmpsgt:
    case Opcode::Cmpsge:
    case Opcode::Cmpult:
    case Opcode::Cmpule:
    case Opcode::Cmpugt:
    case Opcode::Cmpuge:
    case Opcode::Cmpolt:
    case Opcode::Cmpole:
    case Opcode::Cmpogt:
    case Opcode::Cmpoge:
    case Opcode::Cmpunlt:
    case Opcode::Cmpunle:
    case Opcode::Cmpungt:
    case Opcode::Cmpunge:
        return true;
    default:
        return false;
    }
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
    if (pParent) pParent->add_slot(this);
};

Function::Function(const std::string& name) : name(name) {}

Function::~Function() {
    for (auto slot : stack) delete slot;
    stack.clear();
}

StackSlot* Function::get_slot(const std::string& name) {
    for (auto slot : stack)
        if (slot->get_name() == name) return slot;

    return nullptr;
}

u32 Function::get_stack_size() const {
    if (stack.empty())
        return 0;

    return stack.back()->get_offset();
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
