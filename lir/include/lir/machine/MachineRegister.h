//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LIR_MACHINE_REGISTER_H_
#define LIR_MACHINE_REGISTER_H_

#include "lir/machine/Register.h"

#include <cassert>

namespace lir {

/// A register operand to a machine instruction.
///
/// This class does more than wrap over the Register class. It does so for the sake of modelling 
/// some basic register context that helps inform the register allocator how to deal with physical 
/// registers that pop up.
class MachineRegister final {
    /// The underlying register id (virtual or physical).
    Register m_reg;

    /// Optionally, a subregister byte for relevant architectures.
    uint8_t m_subreg = 0;

    /// If true, then this register is the definition of a value. Otherwise, it's the use of a value.
    bool m_def_or_use;

    /// If true, then this register operand is implicit, and does not appear in the final assembly.
    ///
    /// Implicit operands exist to model side effects of operands which do not appear in the opcode.
    bool m_impl;

    /// If true, then this register is expired, and whose value is not used thereafter.
    bool m_expired;

public:
    MachineRegister() = default;

    MachineRegister(Register reg, uint8_t subreg = 0, bool def_or_use = false, 
                    bool implicit = false, bool expired = false)
      : m_reg(reg), m_subreg(subreg), m_def_or_use(def_or_use), m_impl(implicit), 
        m_expired(expired) {}

    /// Returns the register id of this operand.
    Register reg() const { return m_reg; }

    /// Returns the subregister byte of this operand.
    uint8_t subreg() const { return m_subreg; }

    /// Test if this register is a def.
    bool isDef() const { return m_def_or_use; }

    /// Test if this register is a use.
    bool isUse() const { return !m_def_or_use; }
    
    /// Test if this register operand is implicit.
    bool isImplicit() const { return m_impl; }

    /// Test if this register operand is explicit.
    bool isExplicit() const { return !m_impl; }

    /// Test if this register operand is an expiration.
    bool isExpired() const { return m_expired; }

    MachineRegister& setReg(Register reg) {
        m_reg = reg;
        return *this;
    }

    MachineRegister& setSubreg(uint8_t subreg) {
        m_subreg = subreg;
        return *this;
    }

    MachineRegister& setIsDef() {
        m_def_or_use = true;
        return *this;
    }

    MachineRegister& setIsUse() {
        m_def_or_use = false;
        return *this;
    }

    MachineRegister& setIsImplicit() {
        m_impl = true;
        return *this;
    }

    MachineRegister& setIsExplicit() {
        m_impl = false;
        return *this;
    }

    MachineRegister& setIsExpired() {
        m_expired = true;
        return *this;
    }
};

} // namespace lir

#endif // LIR_MACHINE_REGISTER_H_
