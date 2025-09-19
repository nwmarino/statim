#ifndef STATIM_SIIR_MACHINE_INST_H_
#define STATIM_SIIR_MACHINE_INST_H_

#include "siir/machine_operand.hpp"
#include "siir/machine_register.hpp"
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
    /// TODO: Pack this into a MachineOperand* array, and optimize allocations
    /// to reduce on instruction size.
    std::vector<MachineOperand> m_operands;

public:
    MachineInst(u32 opcode, const std::vector<MachineOperand>& operands = {}, 
                MachineBasicBlock* parent = nullptr);

    /// Returns the opcode of this machine instruction.
    u32 opcode() const { return m_opcode; }

    /// Returns the parent basic block of this machine instruction, if there
    /// is a parent.
    const MachineBasicBlock* get_parent() const { return m_parent; }
    MachineBasicBlock* get_parent() { return m_parent; }

    /// Clear the parent link of this machine instruction. Does not detach this
    /// instruction from any existing parent block.
    void clear_parent() { m_parent = nullptr; }

    /// Set the parent basic block of this instruction to |mbb|.
    void set_parent(MachineBasicBlock* mbb) { m_parent = mbb; }

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
    const auto defs() const {
        return m_operands | std::views::filter([](const MachineOperand& mo) { 
            return mo.is_reg() && mo.is_def() && !mo.is_implicit(); 
        });
    }

    auto defs() {
        return m_operands | std::views::filter([](MachineOperand& mo) { 
            return mo.is_reg() && mo.is_def() && !mo.is_implicit(); 
        });
    }

    /// Returns a list of all explicit use operands.
    const auto uses() const {
        return m_operands | std::views::filter([](const MachineOperand& mo) { 
            return mo.is_reg() && mo.is_use() && !mo.is_implicit(); 
        });
    }

    auto uses() {
        return m_operands | std::views::filter([](MachineOperand& mo) { 
            return mo.is_reg() && mo.is_use() && !mo.is_implicit(); 
        });
    }

    /// Returns a list of all explicit & implicit def operands.
    const auto all_defs() const {
        return m_operands | std::views::filter([](const MachineOperand& mo) {
            return mo.is_reg() && mo.is_def();
        });
    }

    auto all_defs() {
        return m_operands | std::views::filter([](MachineOperand& mo) {
            return mo.is_reg() && mo.is_def();
        });
    }

    /// Returns a list of all explicit & implicit use operands.
    const auto all_uses() const {
        return m_operands | std::views::filter([](const MachineOperand& mo) {
            return mo.is_reg() && mo.is_use();
        });
    }

    auto all_uses() {
        return m_operands | std::views::filter([](MachineOperand& mo) {
            return mo.is_reg() && mo.is_use();
        });
    }

    /// Add a new operand |op| to this instruction.
    MachineInst& add_operand(const MachineOperand& op) {
        m_operands.push_back(op);
        return *this;
    }

    MachineInst& add_reg(MachineRegister reg, u16 subreg, bool is_def, 
                 bool is_implicit = false, bool is_kill = false, 
                 bool is_dead = false) {
        m_operands.push_back(MachineOperand::create_reg(
            reg, subreg, is_def, is_implicit, is_kill, is_dead));
        return *this;
    }

    MachineInst& add_mem(MachineRegister reg, i32 disp) {
        m_operands.push_back(MachineOperand::create_mem(reg, disp));
        return *this;
    }

    MachineInst& add_stack_index(u32 idx) {
        m_operands.push_back(MachineOperand::create_stack_index(idx));
        return *this;
    }

    MachineInst& add_imm(i64 imm) {
        m_operands.push_back(MachineOperand::create_imm(imm));
        return *this;
    }

    MachineInst& add_zero() {
        m_operands.push_back(MachineOperand::create_imm(0));
        return *this;
    }

    MachineInst& add_block(MachineBasicBlock* mbb) {
        m_operands.push_back(MachineOperand::create_block(mbb));
        return *this;
    }

    MachineInst& add_constant_index(u32 idx) {
        m_operands.push_back(MachineOperand::create_constant_index(idx));
        return *this;
    }

    MachineInst& add_symbol(const char* symbol) {
        m_operands.push_back(MachineOperand::create_symbol(symbol));
        return *this;
    }

    MachineInst& add_symbol(const std::string& symbol) {
        m_operands.push_back(MachineOperand::create_symbol(symbol.c_str()));
        return *this;
    }
};

} // namespace stm::siir

#endif // STATIM_SIIR_MACHINE_INST_H_
