#ifndef STATIM_SIIR_MACHINE_REGISTER_H_
#define STATIM_SIIR_MACHINE_REGISTER_H_

#include "types/types.hpp"

namespace stm::siir {

/// Potential physical register classes.
///
/// Used in tandem with virtual registers for register allocation.
enum RegisterClass : u8 {
    GeneralPurpose, FloatingPoint, Vector,
};

/// Represents a virtual or physical register.
class MachineRegister final {
public:
    static constexpr u32 NoRegister = 0u;
    static constexpr u32 PhysicalBarrier = 1u;
    static constexpr u32 VirtualBarrier = 1u << 31;

private:
    /// The number of this register.
    ///
    /// All registers share a global namespace, and the value about barriers
    /// determine if the register is physical or virtual.
    ///
    /// 0               Non-register
    /// [1, 2^31)       Physical registers
    /// [2^31, 2^32)    Virtual registers
    u32 m_reg;

public:
    MachineRegister() = default;
    MachineRegister(u32 reg) : m_reg(reg) {}

    /// Returns true if this register is valid.
    bool is_valid() const { return m_reg != NoRegister; }

    /// Returns true if this register is physical.
    bool is_physical() const { 
        return PhysicalBarrier <= m_reg && m_reg < VirtualBarrier; 
    }

    /// Returns true if this register is virtual.
    bool is_virtual() const { return m_reg >= VirtualBarrier; }

    u32 id() const { return m_reg; }

    bool operator == (const MachineRegister& other) const {
        return m_reg == other.m_reg;
    }

    bool operator != (const MachineRegister& other) const {
        return m_reg != other.m_reg;
    }

    bool operator == (u32 other) const {
        return m_reg == other;
    }

    bool operator != (u32 other) const {
        return m_reg != other;
    }

    static bool is_valid(u32 reg) { return reg != NoRegister; }

    static bool is_physical(u32 reg) {
        return PhysicalBarrier <= reg && reg < VirtualBarrier;
    }

    static bool is_virtual(u32 reg) { return reg >= VirtualBarrier; }
};

} // namespace stm::siir

#endif // STATIM_SIIR_MACHINE_REGISTER_H_
