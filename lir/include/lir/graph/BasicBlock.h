//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LOVELACE_IR_BASICBLOCK_H_
#define LOVELACE_IR_BASICBLOCK_H_

//
//  This header file declares the BasicBlock class, which is used to organize
//  a list of instructions within a function.
//

#include "lir/graph/Instruction.h"

#include <cassert>
#include <vector>

namespace lir {

class BasicBlock;
class Function;

/// A basic block is a flat list of instructions that, when well-formed, has
/// exactly one entry point, and one exit point (the sole terminator).
class BasicBlock final {
public:
    using Preds = std::vector<BasicBlock*>;
    using Succs = std::vector<BasicBlock*>;

private:
    Function *m_parent;
    BasicBlock *m_prev = nullptr;
    BasicBlock *m_next = nullptr;
    Instruction *m_head = nullptr;
    Instruction *m_tail = nullptr;
    Preds m_preds = {};
    Succs m_succs = {};

    BasicBlock(Function *parent) : m_parent(parent) {}

public:
    /// Create a new basic block. If the |parent| argument is provided, the new
    /// block will be automatically append to it.
    [[nodiscard]] static BasicBlock *create(Function *parent = nullptr);

    ~BasicBlock();

    BasicBlock(const BasicBlock&) = delete;
    void operator=(const BasicBlock&) = delete;

    BasicBlock(BasicBlock&&) noexcept = delete;
    void operator=(BasicBlock&&) noexcept = delete;

    void set_parent(Function *function) { m_parent = function; }
    const Function *get_parent() const { return m_parent; }
    Function *get_parent() { return m_parent; }

    /// Test if this basic block belongs to a parent function.
    bool has_parent() const { return m_parent != nullptr; }

    /// Detach this basic block from its parent function. 
    /// 
    /// Does not free any memory allocated for this block.
    void detach();

    /// Append this basic block to the back of the given |func|. 
    /// Fails if this block already belongs to a function.
    void append_to(Function *func);

    /// Insert this basic block before the given |block|. 
    /// Fails if this block already belongs to a function.
    void insert_before(BasicBlock *block);

    /// Insert this basic block after the given |block|. 
    /// Fails if this block already belongs to a function.
    void insert_after(BasicBlock *block);

    /// Remove the given |inst| from this basic block, if it belongs.
    void remove(Instruction *inst);

    /// Test if this block is the first one in its parent function.
    inline bool is_entry() const { 
        return m_parent != nullptr && m_prev == nullptr; 
    }

    void set_prev(BasicBlock *block) { m_prev = block; }
    const BasicBlock *get_prev() const { return m_prev; }
    BasicBlock *get_prev() { return m_prev; }

    void set_next(BasicBlock *block) { m_next = block; }
    const BasicBlock *get_next() const { return m_next; }
    BasicBlock *get_next() { return m_next; }

    void set_head(Instruction *inst) { m_head = inst; }
    const Instruction *get_head() const { return m_head; }
    Instruction *get_head() { return m_head; }

    void set_tail(Instruction *inst) { m_tail = inst; }
    const Instruction *get_tail() const { return m_tail; }
    Instruction *get_tail() { return m_tail; }

    /// Prepend the given |inst| to the front this block.
    void prepend(Instruction *inst);

    /// Append the given |inst| to the back of this block.
    void append(Instruction *inst);

    /// Insert the given |inst| into this block at position |i|.
    void insert(Instruction *inst, uint32_t i);

    /// Insert the given |inst| into this basic block immediately after 
    /// |insert_after|. 
    /// Fails if |inst| already belongs to another block, or if |insert_after| 
    /// is not part of this block.
    void insert(Instruction *inst, Instruction *insert_after);

    /// Test if this block has no instructions.
    inline bool empty() const { return m_head == nullptr; }

    /// Returns the size of this block by the number of instructions in it.
    uint32_t size() const;

    /// Returns the numeric position of this block in its parent function.
    uint32_t position() const;

    /// Returns the list of blocks that are predecessors to this one.
    const Preds &get_preds() const { return m_preds; }
    Preds &get_preds() { return m_preds; }

    /// Returns the number of blocks that are predecessors to this one.
    uint32_t num_preds() const { return m_preds.size(); }

    /// Test if this block has any predecessors.
    bool has_preds() const { return !m_preds.empty(); }

    /// Returns the |i|-th predecessor block.
    const BasicBlock *get_pred(uint32_t i) const {
        assert(i < num_preds() && "index out of bounds!");
        return m_preds[i];
    }

    BasicBlock *get_pred(uint32_t i) {
        assert(i < num_preds() && "index out of bounds!");
        return m_preds[i];
    }

    /// Add the given |block| as a predecessor to this one.
    void add_pred(BasicBlock *block) {
        // @Todo: Check that the block isn't already a predecessor?
        m_preds.push_back(block);
    }

    /// Returns the list of blocks that are successors to this one.
    const Preds &get_succs() const { return m_succs; }
    Preds &get_succs() { return m_succs; }

    /// Returns the number of blocks that are successors to this one.
    uint32_t num_succs() const { return m_succs.size(); }

    /// Test if this block has any successors.
    bool has_succs() const { return !m_succs.empty(); }

    /// Returns the |i|-th successor block.
    const BasicBlock *get_succ(uint32_t i) const {
        assert(i < num_succs() && "index out of bounds!");
        return m_succs[i];
    }

    BasicBlock *get_succ(uint32_t i) {
        assert(i < num_succs() && "index out of bounds!");
        return m_succs[i];
    }

    /// Add the given |block| as a successor to this one.
    void add_succ(BasicBlock *block) {
        // @Todo: Check that the block isn't already a successor?
        m_succs.push_back(block);
    }

    /// Test if this basic block contains a terminating instruction.
    bool terminates() const;

    /// Returns the number of terminating instructions in this basic block.
    uint32_t num_terminators() const;
    
    /// Returns the earliest terminating instruction in this basic block, if 
    /// one exists.
    const Instruction *terminator() const;
    Instruction *terminator() {
        return const_cast<Instruction*>(
            static_cast<const BasicBlock*>(this)->terminator());
    }

    void print(std::ostream &os, PrintPolicy policy) const;
};

} // namespace lir

#endif // LOVELACE_IR_BASICBLOCK_H_
