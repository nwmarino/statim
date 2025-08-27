#ifndef STATIM_SIIR_TARGET_HPP_
#define STATIM_SIIR_TARGET_HPP_

#include "siir/type.hpp"
#include "types/types.hpp"

#include <unordered_map>

namespace stm {

namespace siir {

class Target final {
public:
    enum Arch : u8 {
        amd64,
    };

    enum ABI : u8 {
        SystemV,
    };

    enum OS : u8 {
        Linux,
    };

private:
    Arch m_arch;
    ABI m_abi;
    OS m_os;

    bool m_little_endian;
    u32 m_ptr_size;
    u32 m_ptr_align;

    struct LayoutRule final {
        u32 size;
        u32 abi_align;
    };

    std::unordered_map<Type::Kind, LayoutRule> m_rules = {};
    
public:
    Target(Arch arch, ABI abi, OS os);

    Arch arch() const { return m_arch; }
    ABI abi() const { return m_abi; }
    OS os() const { return m_os; }

    u32 get_type_size(const Type* type) const;
    u32 get_type_align(const Type* type) const;

    u32 get_pointer_size() const { return m_ptr_size; }
    u32 get_pointer_align() const { return m_ptr_align; }

    bool is_little_endian() const { return m_little_endian; }
    bool is_big_endian() const { return !m_little_endian; }

    bool is_scalar_type(const Type* type) const;

    u32 get_element_offset(const ArrayType* type, u32 idx) const;
    u32 get_pointee_offset(const PointerType* type, u32 idx) const;
    u32 get_field_offset(const StructType* type, u32 idx) const;
};

} // namespace siir

} // namespace stm

#endif // STATIM_SIIR_TARGET_HPP_
