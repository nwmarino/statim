#ifndef STATIM_MACHINE_INST_HPP_
#define STATIM_MACHINE_INST_HPP_

#include "machine/operand.hpp"
#include "types.hpp"

#include <cassert>
#include <vector>

namespace stm {

class MachineFunction;

/// Represents a target-dependent machine instruction.
class MachineInst final {
    /// The opcode of this instruction.
    u32 m_opcode;

    /// The parent basic block of this instruction, if it exists.
    MachineBasicBlock* m_parent;
    
    /// The operands of this instruction.
    ///
    /// TODO: Pack this into a MachineOperand* array, and better manage
    /// allocations.
    std::vector<MachineOperand> m_operands;

public:
    MachineInst(
        u32 opcode, 
        const std::vector<MachineOperand>& operands, 
        MachineBasicBlock* parent = nullptr);

    u32 opcode() const { return m_opcode; }

    const MachineBasicBlock* get_parent() const { return m_parent; }
    MachineBasicBlock* get_parent() { return m_parent; }

    const MachineFunction* get_mf() const;
    MachineFunction* get_mf();

    /// \returns The position of this instruction relative to others in the
    /// parent function.
    u32 position() const;

    /// \returns The raw number of operands included in this instruction.
    u32 num_operands() const { return m_operands.size(); }

    const MachineOperand& get_operand(u32 idx) const {
        assert(idx <= num_operands());
        return m_operands[idx];
    }

    MachineOperand& get_operand(u32 idx) { 
        assert(idx <= num_operands());
        return m_operands[idx];
    }

    u32 num_defs() const;

    u32 num_implicit_operands() const;

    u32 num_explicit_operands() const {
        return num_operands() - num_implicit_operands();
    }

    u32 num_implicit_defs() const;

    u32 num_explicit_defs() const;

    bool has_implicit_def() const;

    /// \returns A list of all explicit def operands.
    std::vector<const MachineOperand&> defs() const;

    /// \returns A list of all explicit use operands.
    std::vector<const MachineOperand&> uses() const;

    /// \returns A list of all explicit & implicit def operands.
    std::vector<const MachineOperand&> all_defs() const;

    /// \returns A list of all explicit & implicit use operands.
    std::vector<const MachineOperand&> all_uses() const;

    /// Add a new operand \p mo to this instruction.
    void add_operand(const MachineOperand& mo) {
        m_operands.push_back(mo);
    }

    void print(std::ostream& os) const;
};

} // namespace stm

#endif // STATIM_MACHINE_INST_HPP_
