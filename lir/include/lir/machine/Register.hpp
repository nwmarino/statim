//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LIR_REGISTER_H_
#define LIR_REGISTER_H_

#include <cstdint>

namespace lir {

/// The different kinds of classes for physical registers.
///
/// Used for the sake of informing register allocation how to allocate for 
/// virtual registers without high-level type information.
enum class RegisterClass : uint32_t {
    GeneralPurpose, FloatingPoint,
};

/// Represents a register in the machine IR.
///
/// Such a register may be virtual or physical (named), distinguished by a
/// numeric barrier.
class Register final {
public:
    static constexpr uint32_t NO_REGISTER = 0u;
    static constexpr uint32_t PHYSICAL_BARRIER = 1u;
    static constexpr uint32_t VIRTUAL_BARRIER = 1u << 31;

    /// Test if the given register |id| is valid.
    static inline bool is_valid(uint32_t id) { 
        return id != NO_REGISTER;
    }

    /// Test if the given register |id| signifies a physical register.
    static inline bool is_physical(uint32_t id) {
        return PHYSICAL_BARRIER <= id && id < VIRTUAL_BARRIER;
    }

    /// Test if the given register |id| signifies a virtual register.
    static inline bool is_virtual(uint32_t id) { 
        return id >= VIRTUAL_BARRIER; 
    }

private:
    /// The id of this register.
    ///
    /// All registers share a global namespace, and the value about the
    /// barriers determine if the register is physical or virtual.
    ///
    /// 0               Sentinel/none
    /// [1, 2^31)       Physical registers
    /// [2^31, 2^32)    Virtual registers
    uint32_t m_id;

    /// The class of this register.
    RegisterClass m_cls = RegisterClass::GeneralPurpose;

public:
    Register() = default;
    Register(uint32_t id) : m_id(id) {}
    Register(uint32_t id, RegisterClass cls) : m_id(id), m_cls(cls) {}

    bool operator==(uint32_t other) const { 
        return m_id == other; 
    }
    
    bool operator==(const Register &other) const {
        return m_id == other.m_id;
    }

    /// Test if this register is valid.
    inline bool is_valid() const { 
        return Register::is_valid(m_id); 
    }

    /// Test if this register is physical.
    inline bool is_physical() const { 
        return Register::is_physical(m_id);
    }

    /// Test if this register is virtual.
    inline bool is_virtual() const {
        return Register::is_virtual(m_id);
    }

    /// Returns the id of this register.
    uint32_t id() const { return m_id; }

    /// Returns the class of this register.
    RegisterClass cls() const { return m_cls; }
};

} // namespace lir

#endif // LIR_REGISTER_H_
