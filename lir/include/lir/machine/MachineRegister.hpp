//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LIR_MACHINE_REGISTER_H_
#define LIR_MACHINE_REGISTER_H_

#include "lir/machine/Register.hpp"

#include <cassert>

namespace lir {

/// A register operand to a machine instruction.
///
/// This class does more than wrap over the Register class. It does so for the 
/// sake of modelling some basic register context that helps inform the 
/// register allocator how to deal with physical registers that pop up.
class MachineRegister final {
public:
    enum class Kind : uint8_t {
        Use = 0,
        Def = 1,
    };

private:
    /// The underlying register id (virtual or physical).
    Register m_reg;

    /// Optionally, a subregister byte for relevant architectures.
    uint8_t m_subreg = 0;

    /// The kind of register operand this is i.e. use of a value or definition 
    /// thereof.
    Kind m_kind;

    /// If true, then this register operand is implicit, and should not 
    /// appear in the final assembly.
    ///
    /// Implicit operands simply exist to model side effects of operands which
    /// do not appear plainly.
    bool m_impl;

    /// If true and this register operand is a use, then this marks the killing
    /// of a register i.e. the last registered use for a given value.
    ///
    /// If true and this register operand is a def, then this marks the death
    /// of a register i.e. it will not be used after the definition.
    ///
    /// If false, then this flag means nothing under any circumstances.
    ///
    /// Kill/dead help model when instructions (over)write registers, often
    /// implicitly, and how register allocation should interpret the liveness.
    bool m_kill_or_dead;

public:
    MachineRegister(Register reg, uint8_t subreg = 0, Kind kind = Kind::Use, 
                    bool impl = false, bool kill_or_dead = false)
      : m_reg(reg), m_subreg(subreg), m_kind(kind), m_impl(impl), 
        m_kill_or_dead(kill_or_dead) {}

    void set_register(Register reg) { m_reg = reg; }
    Register get_register() const { return m_reg; }

    void set_subreg(uint8_t subreg) { m_subreg = subreg; }
    uint8_t get_subreg() const { return m_subreg; }

    /// Test if this register is a use.
    inline bool is_use() const { return m_kind == Kind::Use; }

    /// Test if this register is a def.
    inline bool is_def() const { return m_kind == Kind::Def; }

    /// Mark this register operand as being implicit.
    inline void set_implicit() { m_impl = true; }

    /// Mark this register operand as being explicit.
    inline void set_explicit() { m_impl = false; }
    
    /// Test if this register operand is implicit.
    inline bool is_implicit() const { return m_impl; }

    void set_is_kill(bool value = true) { 
        assert(is_use() && "cannot mark a register def as killed!");
        m_kill_or_dead = value; 
    }
    
    void set_is_dead(bool value = true) {
        assert(is_def() && "cannot mark a register use as dead!");
        m_kill_or_dead = value;
    }

    /// Test if this register use is kills the value in it.
    inline bool is_kill() const { return m_kill_or_dead; }

    /// Test if this register def is dead.
    inline bool is_dead() const { return m_kill_or_dead; }
};

} // namespace lir

#endif // LIR_MACHINE_REGISTER_H_
