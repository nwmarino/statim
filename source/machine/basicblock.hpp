#ifndef STATIM_SIIR_MACHINE_BASICBLOCK_H_
#define STATIM_SIIR_MACHINE_BASICBLOCK_H_

#include "machine/machine_inst.hpp"

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
    MachineBasicBlock(
        const BasicBlock* bb, 
        MachineFunction* parent = nullptr);

    /// \returns The corresponding bytecode basic block this machine block
    /// derives from, if it exists.
    const BasicBlock* get_basic_block() const { return m_bb; }

    const MachineFunction* get_parent() const { return m_parent; }
    MachineFunction* get_parent() { return m_parent; }

    /// \returns The position of this block in its parent function.
    u32 position() const;

    bool empty() const { return m_insts.empty(); }

    /// \returns The number of instructions in this block.
    u32 size() const { return m_insts.size(); }

    const MachineInst& front() const { return m_insts.front(); }
    MachineInst& front() { return m_insts.front(); }

    const MachineInst& back() const { return m_insts.back(); }
    MachineInst& back() { return m_insts.back(); }

    const std::vector<MachineInst>& insts() const { return m_insts; }

    const MachineBasicBlock* prev() const { return m_prev; }
    MachineBasicBlock* prev() { return m_prev; }

    const MachineBasicBlock* next() const { return m_next; }
    MachineBasicBlock* next() { return m_next; }

    void set_prev(MachineBasicBlock* prev) { m_prev = prev; }
    void set_next(MachineBasicBlock* next) { m_next = next; }

    void push_front(const MachineInst& inst) {
        m_insts.insert(m_insts.begin(), inst);
    }

    void push_back(const MachineInst& inst) {
        m_insts.push_back(inst);
    }
};

} // namespace stm::siir

#endif // STATIM_SIIR_MACHINE_BASICBLOCK_H_
