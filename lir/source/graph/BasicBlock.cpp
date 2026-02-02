//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lir/graph/BasicBlock.hpp"
#include "lir/graph/Function.hpp"
#include "lir/graph/Instruction.hpp"

#include <format>

using namespace lir;

BasicBlock::~BasicBlock() {
    Instruction *curr = m_head;
    while (curr) {
        Instruction* tmp = curr->get_next();
        delete curr;
        curr = tmp;
    }

    m_head = nullptr, m_tail = nullptr;
    m_prev = nullptr, m_next = nullptr;
    m_preds.clear();
    m_succs.clear();
}

BasicBlock *BasicBlock::create(Function *parent) {
    BasicBlock* block = new BasicBlock(nullptr);
    assert(block);
    
    if (parent)
        parent->append(block);

    return block;
}

void BasicBlock::detach() {
    assert(m_parent && "block does not belong to a function!");

    m_parent->remove(this); // Updates parent pointer for us.
}

void BasicBlock::append_to(Function *parent) {
    assert(parent && "parent cannot be null!");
    assert(!has_parent() && "block already belongs to a function!");

    parent->append(this);
}

void BasicBlock::insert_before(BasicBlock *block) {
    assert(block && "block cannot be null!");
    assert(!has_parent() && "block already belongs to a function!");

    m_prev = block->m_prev;
    m_next = block;

    if (block->m_prev)
        block->m_prev->m_next = this;

    block->m_prev = this;
    m_parent = block->get_parent();
}

void BasicBlock::insert_after(BasicBlock* block) {
    assert(block && "block cannot be null!");
    assert(!m_parent && "block already belongs to a function!");

    m_prev = block;
    m_next = block->get_next();

    if (block->get_next())
        block->get_next()->set_prev(this);

    block->set_next(this);
    m_parent = block->get_parent();
}

void BasicBlock::remove(Instruction *inst) {
    assert(inst && "instruction cannot be null!");
    assert(inst->get_parent() == this && 
        "instruction does not belong to this block!");

    // Update the next pointer for the instruction before the one to remove.
    if (inst->get_prev()) {
        inst->get_prev()->set_next(inst->get_next());
    } else {
        if (inst->get_next()) {
            // Instruction is at the front of the block, so update the head.
            m_head = inst->get_next();
        } else {
            // Instruction was the last in the block.
            m_head = nullptr, m_tail = nullptr;
        }
    }

    // Update the previous pointer for the instruction after the one to remove.
    if (inst->get_next()) {
        inst->get_next()->set_prev(inst->get_prev());
    } else {
        if (inst->get_prev()) {
            // Instruction is at the back of the block, so update the tail.
            m_tail = inst->get_prev();
        } else {
            // Instruction was the last in the block.
            m_head = m_tail = nullptr;
        }
    }

    inst->set_prev(nullptr);
    inst->set_next(nullptr);
    inst->set_parent(nullptr);
}

void BasicBlock::prepend(Instruction *inst) {
    assert(inst && "instruction cannot be null!");

    if (m_head) {
        inst->set_next(m_head);
        m_head->set_prev(inst);
        m_head = inst;
    } else {
        m_head = inst, m_tail = inst;
    }

    inst->set_parent(this);
}

void BasicBlock::append(Instruction *inst) {
    assert(inst && "instruction cannot be null!");

    if (m_tail) {
        inst->set_prev(m_tail);
        m_tail->set_next(inst);
        m_tail = inst;
    } else {
        m_head = inst, m_tail = inst;
    }

    inst->set_parent(this);
}

void BasicBlock::insert(Instruction *inst, uint32_t i) {
    assert(inst && "instruction cannot be null!");
    
    uint32_t position = 0;
    for (Instruction *curr = m_head; curr; curr = curr->get_next())
        if (position++ == i) 
            return inst->insert_before(curr);

    append(inst);
}

void BasicBlock::insert(Instruction *inst, Instruction *insert_after) {
    inst->insert_after(insert_after);
}

uint32_t BasicBlock::size() const {
    uint32_t size = 0;
    for (const Instruction *curr = m_head; curr; curr = curr->get_next())
        size++;

    return size;
}

uint32_t BasicBlock::position() const {
    assert(m_parent && "block does not belong to a function!");

    uint32_t num = 0;

    // Count backwards from where the block is relative to the first block in 
    // the parent function, i.e. the one with no previous block.
    for (const BasicBlock *curr = m_parent->get_head(); curr; curr = curr->get_next()) {
        if (curr == this)
            return num;
        
        num++;
    }
    
    assert(false && "block has a parent, but missing from block list!");
}

bool BasicBlock::terminates() const {
    // Start at the back of the block and move to the front, since terminators
    // are most likely to be towards the end.
    for (const Instruction *curr = m_tail; curr; curr = curr->get_prev()) {
        if (curr->is_terminator()) 
            return true;
    }

    return false;
}

uint32_t BasicBlock::num_terminators() const {
    uint32_t num = 0;
    for (const Instruction *curr = m_head; curr; curr = curr->get_next()) {
        if (curr->is_terminator()) 
            num++;
    }

    return num;
}

const Instruction *BasicBlock::terminator() const {
    for (const Instruction *curr = m_head; curr; curr = curr->get_next()) {
        if (curr->is_terminator()) 
            return curr;
    }

    return nullptr;
}

void BasicBlock::print(std::ostream &os, PrintPolicy policy) const {
    if (policy == PrintPolicy::Use) {
        os << std::format("bb{}", position());
    } else if (policy == PrintPolicy::Def) {
        os << std::format("bb{}:\n", position());

        const Instruction *curr = m_head;
        while (curr) {
            os << '\t';
            curr->print(os, PrintPolicy::Def);
            curr = curr->get_next();
        }
    }
}
