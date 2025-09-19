#ifndef STATIM_SIIR_MACHINE_BASICBLOCK_H_
#define STATIM_SIIR_MACHINE_BASICBLOCK_H_

#include "siir/machine_inst.hpp"

#include <cassert>
#include <vector>

namespace stm::siir {

class BasicBlock;

/// Represents a target-dependent basic block, derived from a bytecode block.
class MachineBasicBlock final {
    friend class MachineFunction;

    /// The bytecode block this basic block derives from.
    const BasicBlock* m_bb;

    /// The parent function of this basic block.
    MachineFunction* m_parent;

    /// The instructions in this block.
    std::vector<MachineInst> m_insts;

    /// Links to the previous and next block in the parent function.
    MachineBasicBlock* m_prev = nullptr;
    MachineBasicBlock* m_next = nullptr;

public:
    MachineBasicBlock(const BasicBlock* bb, MachineFunction* parent = nullptr);

    /// Returns the corresponding SIIR basic block this block derives from, 
    /// if it exists.
    const BasicBlock* get_basic_block() const { return m_bb; }

    /// Returns the parent function to this basic block.
    const MachineFunction* get_parent() const { return m_parent; }
    MachineFunction* get_parent() { return m_parent; }

    /// Clears the parent link of this basic block. Does not detach it from
    /// any existing parent function.
    void clear_parent() { m_parent = nullptr; }

    /// Set the parent function of this basic block to |mf|.
    void set_parent(MachineFunction* mf) { m_parent = mf; }

    /// Returns the position of this block relative to other blocks in its
    /// parent function.
    u32 position() const;

    /// Returns true if this basic block has no instructions.
    bool empty() const { return m_insts.empty(); }

    /// Returns the number of instructions in this block.
    u32 size() const { return m_insts.size(); }

    const MachineInst& front() const { return m_insts.front(); }
    MachineInst& front() { return m_insts.front(); }

    const MachineInst& back() const { return m_insts.back(); }
    MachineInst& back() { return m_insts.back(); }

    const std::vector<MachineInst>& insts() const { return m_insts; }
    std::vector<MachineInst>& insts() { return m_insts; }

    const MachineBasicBlock* prev() const { return m_prev; }
    MachineBasicBlock* prev() { return m_prev; }

    const MachineBasicBlock* next() const { return m_next; }
    MachineBasicBlock* next() { return m_next; }

    void set_prev(MachineBasicBlock* prev) { m_prev = prev; }
    void set_next(MachineBasicBlock* next) { m_next = next; }

    /// Append |inst| to the back of this basic block.
    void push_front(MachineInst& inst) {
        inst.set_parent(this);
        m_insts.insert(m_insts.begin(), inst);
    }

    /// Prepend |inst| to the front of this basic block.
    void push_back(MachineInst& inst) {
        inst.set_parent(this);
        m_insts.push_back(inst);
    }

    void insert(MachineInst& inst, u32 i) {
        assert(i < size());
        inst.set_parent(this);
        m_insts.insert(m_insts.begin() + i, inst);
    }
};

} // namespace stm::siir

#endif // STATIM_SIIR_MACHINE_BASICBLOCK_H_
