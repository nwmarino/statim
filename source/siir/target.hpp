#ifndef STATIM_SIIR_TARGET_HPP_
#define STATIM_SIIR_TARGET_HPP_

#include "siir/type.hpp"
#include "types/types.hpp"

#include <unordered_map>

namespace stm {
namespace siir {

/// A backend target, used for various code generation and type information.
class Target final {
public:
    /// Recognized CPU architectures.
    enum Arch : u8 {
        x64,
    };

    /// Recognized application binary interfaces.
    enum ABI : u8 {
        SystemV,
    };

    /// Recognized operating systems.
    enum OS : u8 {
        Linux,
    };

private:
    /// The architecture of this target.
    Arch m_arch;

    /// The ABI of this target.
    ABI m_abi;

    /// The operating system of this target.
    OS m_os;

    /// If true, this target architecture is little endian (LSB is at lowest
    /// address), and if false, this target is big endian.
    bool m_little_endian;

    /// The pointer size and alignment of this target in bits.
    u32 m_ptr_size;
    u32 m_ptr_align;

    /// A layout rule for a type.
    struct LayoutRule final {
        u32 size_in_bits;
        u32 abi_align;
    };

    /// Default type layout rules.
    std::unordered_map<Type::Kind, LayoutRule> m_rules = {};
    
public:
    /// Create a new target.
    Target(Arch arch, ABI abi, OS os);

    /// Returns the architecture of this target.
    Arch arch() const { return m_arch; }

    /// Returns the ABI of this target.
    ABI abi() const { return m_abi; }

    /// Returns the operating system of this target.
    OS os() const { return m_os; }

    /// Returns the size of |ty| in bytes.
    u32 get_type_size(const Type* ty) const;

    /// Returns the size of |ty| in bits.
    u32 get_type_size_in_bits(const Type* ty) const;

    /// Returns the natural alignment in bytes for |ty|.
    u32 get_type_align(const Type* ty) const;

    /// Returns the natural alignment in bits for |ty|.
    u32 get_type_align_in_bits(const Type* ty) const;

    /// Returns the target pointer size in bytes.
    u32 get_pointer_size() const { return m_ptr_size / 8; }

    /// Returns the target pointer size in bits.
    u32 get_pointer_size_in_bits() const { return m_ptr_size; }

    /// Returns the target natural pointer alignment in bytes.
    u32 get_pointer_align() const { return m_ptr_align / 8; }

    /// Returns the target natural pointer alignment in bits.
    u32 get_pointer_align_in_bits() const { return m_ptr_align; }

    /// Returns true if this target is little-endian.
    bool is_little_endian() const { return m_little_endian; }
    
    /// Returns true if this target is big-endian.
    bool is_big_endian() const { return !m_little_endian; }

    /// Returns true if |type| is a scalar type, that is, not an aggregate
    /// of values or a complex type. Pointers are considered scalar.
    bool is_scalar_type(const Type* type) const;

    /// Returns the array element offset for |type| at the index |idx|.
    u32 get_element_offset(const ArrayType* type, u32 idx) const;

    /// Returns the pointer pointee offset for |type| at the index |idx|.
    u32 get_pointee_offset(const PointerType* type, u32 idx) const;

    /// Returns the offset of a structure field of |type| at the index |idx|.
    u32 get_field_offset(const StructType* type, u32 idx) const;
};

} // namespace siir
} // namespace stm

#endif // STATIM_SIIR_TARGET_HPP_
