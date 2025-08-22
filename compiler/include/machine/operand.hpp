#ifndef STATIM_MACHINE_OPERAND_HPP_
#define STATIM_MACHINE_OPERAND_HPP_

#include "machine/target.hpp"
#include "types.hpp"

#include <cassert>
#include <ostream>

namespace stm {

class MachineBasicBlock;

/// Represents a target-dependent operand to a machine instruction.
class MachineOperand final {
public:
    enum MachineOperandKind : u8 {
        MO_Register,    ///< Register, physical or virtual.
        MO_Memory,      ///< Memory references on a base register.
        MO_Immediate,   ///< Immediate, less than 64-bits.
        MO_BasicBlock,  ///< Reference to a basic block.
        MO_Symbol,      ///< Reference to named symbol.
    };

private:
    /// Defines the kind of operand this is, discriminating the union.
    u16 m_kind : 4;

    /// subreg - optional subregister for register operands. 0 indicates
    /// no subregister.
    u16 m_subreg : 9;
    
    /// is_def - true if this register operand is a def, false if it is a 
    /// use.
    u16 m_is_def : 1;

    /// is_kill_or_dead - true if a. this operand is a use and is the last 
    /// use of a register or b. this operand is a def and is never used by a
    /// following instruction.
    u16 is_kill_or_dead : 1;

    /// is_implicit - true if this register operand is an implicit def or 
    /// use, false if it is explicit.
    u16 m_is_implicit : 1;

    union {
        /// For MOK_Register operands.
        u32 m_regno;

        /// For MOK_Memory operands.
        struct {
            u32 regno;
            i32 disp;
        } m_mem;

        /// For MOK_Immediate operands.
        i64 m_imm;

        /// For MOK_BasicBlock operands.
        MachineBasicBlock* m_mbb;

        /// For MOK_Symbol operands.
        const char* m_symbol;
    };

public:
    static MachineOperand create_reg(u32 regno, u16 subreg, bool is_def, 
        bool is_implicit = false, bool is_kill = false, bool is_dead = false);

    static MachineOperand create_mem(u32 regno, i32 disp);

    static MachineOperand create_imm(i64 imm);

    static MachineOperand create_block(MachineBasicBlock* mbb);

    static MachineOperand create_symbol(const char* symbol);

    bool is_reg() const { return m_kind == MO_Register; }

    bool is_mem() const { return m_kind == MO_Memory; }

    bool is_imm() const { return m_kind == MO_Immediate; }

    bool is_mmb() const { return m_kind == MO_BasicBlock; }

    bool is_symbol() const { return m_kind == MO_Symbol; }

    u32 get_reg() const {
        assert(is_reg());
        return m_regno;
    }

    u16 get_subreg() const {
        assert(is_reg());
        return m_subreg;
    }

    bool is_def() const { 
        assert(is_reg());
        return m_is_def; 
    }

    bool is_use() const {
        assert(is_reg());
        return !m_is_def; 
    }

    bool is_kill() const {
        assert(is_reg());
        return is_kill_or_dead & !m_is_def;
    }

    bool is_dead() const {
        assert(is_reg());
        return is_kill_or_dead && m_is_def;
    }

    bool is_implicit() const {
        assert(is_reg());
        return m_is_implicit;
    }

    u32 get_mem_base() const {
        assert(is_mem());
        return m_mem.regno;
    }

    u32 get_mem_disp() const {
        assert(is_mem());
        return m_mem.disp;
    }

    i64 get_imm() const {
        assert(is_imm());
        return m_imm;
    }

    MachineBasicBlock* get_mmb() const {
        assert(is_mmb());
        return m_mbb;
    }
    
    const char* get_symbol() const {
        assert(is_symbol());
        return m_symbol;
    }

    void set_reg(u32 regno) {
        assert(is_reg());
        m_regno = regno;
    }

    void set_subreg(u16 subreg) {
        assert(is_reg());
        m_subreg = subreg;
    }

    void set_is_def(bool value = true) {
        assert(is_reg());
        m_is_def = value;
    }

    void set_is_use(bool value = true) {
        assert(is_reg());
        m_is_def = !value;
    }

    void set_is_kill(bool value = true) {
        assert(is_reg());
        assert(is_use());
        is_kill_or_dead = value;
    }

    void set_is_dead(bool value = true) {
        assert(is_reg());
        assert(is_def());
        is_kill_or_dead = value;
    }

    void set_is_implicit(bool value = true) {
        assert(is_reg());
        m_is_implicit = value;
    }

    void set_mem_base(u32 regno) {
        assert(is_mem());
        m_mem.regno = regno;
    }

    void set_mem_disp(i32 disp) {
        assert(is_mem());
        m_mem.disp = disp;
    }

    void set_imm(i64 imm) {
        assert(is_imm());
        m_imm = imm;
    }

    void set_mbb(MachineBasicBlock* mbb) {
        assert(is_mmb());
        m_mbb = mbb;
    }

    void set_symbol(const char* symbol) {
        assert(is_symbol());
        m_symbol = symbol;
    }
};

} // namespace stm

#endif // STATIM_MACHINE_OPERAND_HPP_
