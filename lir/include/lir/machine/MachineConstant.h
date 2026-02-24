//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LIR_MACHINE_CONSTANT_H_
#define LIR_MACHINE_CONSTANT_H_

#include <cassert>
#include <cstdint>

namespace lir {

/// Represents constant data of a certain precision.
class MachineConstant final {
public:
    /// The different kinds of constants.
    enum class Kind : uint32_t {
        Zero,
        Int8, Int16, Int32, Int64,
        Float32, Float64,
    };

private:
    Kind m_kind;
    union {
        long m_zeros;
        long m_int;
        double m_fp;
    };

public:
    MachineConstant(long zeros) : m_kind(Kind::Zero), m_zeros(zeros) {}
    MachineConstant(Kind kind, long value) : m_kind(kind), m_int(value) {}
    MachineConstant(Kind kind, double value) : m_kind(kind), m_fp(value) {}
    
    inline Kind kind() const { return m_kind; }

    /// Test if this is a zero constant.
    inline bool is_zeros() const {
        return m_kind == Kind::Zero;
    }

    /// Test if this is an integer constant.
    inline bool is_int() const { 
        return m_kind == Kind::Int8 || m_kind == Kind::Int16 
            || m_kind == Kind::Int32 || m_kind == Kind::Int64; 
    }

    /// Test if this is a floating point constant.
    inline bool is_fp() const {
        return m_kind == Kind::Float32 || m_kind == Kind::Float64;
    }

    /// Returns the zero count of this constant.
    long get_zeros() const {
        assert(is_zeros() && "this is not a zeros constant!");
        return m_zeros;
    }

    /// Returns the value of this constant as an integer.
    long get_int() const {
        assert(is_int() && "this is not an integer constant!");
        return m_int;
    }

    /// Returns the value of this constant as a float.
    double get_fp() const {
        assert(is_fp() && "this is not a float constant!");
        return m_fp;
    }
};

} // namespace lir

#endif // LIR_MACHINE_CONSTANT_H_
