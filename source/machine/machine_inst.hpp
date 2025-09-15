#ifndef STATIM_SIIR_MACHINE_INST_H_
#define STATIM_SIIR_MACHINE_INST_H_

#include "machine/machine_operand.hpp"
#include "types/types.hpp"

#include <cassert>
#include <ranges>
#include <vector>

namespace stm::siir {

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
    MachineInst(u32 opcode, const std::vector<MachineOperand>& operands, 
                MachineBasicBlock* parent = nullptr);

    /// Returns the opcode of this machine instruction.
    u32 opcode() const { return m_opcode; }

    /// Returns the parent basic block of this machine instruction, if there
    /// is a parent.
    const MachineBasicBlock* get_parent() const { return m_parent; }
    MachineBasicBlock* get_parent() { return m_parent; }

    /// Returns the parent function of this machine instruction, if there is
    /// a parent.
    const MachineFunction* get_mf() const;
    MachineFunction* get_mf();

    /// Returns the raw number of operands included in this instruction.
    u32 num_operands() const { return m_operands.size(); }

    const std::vector<MachineOperand>& operands() const { return m_operands; }
    std::vector<MachineOperand>& operands() { return m_operands; }

    /// Returns the machine operand at position |idx| of this instruction.
    const MachineOperand& get_operand(u32 idx) const {
        assert(idx <= num_operands());
        return m_operands[idx];
    }

    MachineOperand& get_operand(u32 idx) { 
        assert(idx <= num_operands());
        return m_operands[idx];
    }

    /// Returns the number of defining operands part of this instruction.
    u32 num_defs() const;

    /// Returns the number of implicit operands part of this instruction.
    u32 num_implicit_operands() const;

    /// Returns the number of explicit operands part of this instruction.
    u32 num_explicit_operands() const;

    /// Returns the number of implicitly defining operands that are part of
    /// this instruction.
    u32 num_implicit_defs() const;

    /// Returns the number of explicitly defining operands that are part of
    /// this instruction.
    u32 num_explicit_defs() const;

    /// Returns true if any of this instruction's operands are implicit defs.
    bool has_implicit_def() const;

    /// Returns a list of all explicit def operands.
    std::vector<MachineOperand> defs() const;

    /// Returns a list of all explicit use operands.
    auto uses() {
        //return m_operands | std::views::filter([](MachineOperand& mo) { 
        //    return mo.is_reg() && mo.is_use(); 
        //});
    }

    /// Returns a list of all explicit & implicit def operands.
    std::vector<MachineOperand> all_defs() const;

    /// Returns a list of all explicit & implicit use operands.
    std::vector<MachineOperand> all_uses() const;

    /// Add a new operand |op| to this instruction.
    void add_operand(const MachineOperand& op) {
        m_operands.push_back(op);
    }
};

} // namespace stm::siir

#endif // STATIM_SIIR_MACHINE_INST_H_
