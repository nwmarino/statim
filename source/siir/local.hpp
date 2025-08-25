#ifndef STATIM_SIIR_LOCAL_HPP_
#define STATIM_SIIR_LOCAL_HPP_

#include "siir/value.hpp"

namespace stm {

class Function;

class Local final : public Value {
    Function* m_parent;
    const Type* m_alloc_type;
    u32 m_align;

    Local(const Type* alloc_type, u32 align, const Type* type, 
          const std::string& name, Function* parent);

public:
    /// Create a new local with allocation type \p alloc_type and alignment
    /// \p align.
    static Local* create(const Type* alloc_type, u32 align, 
                         const std::string& name = "", 
                         Function* parent = nullptr);

    const Function* get_parent() const { return m_parent; }
    Function* get_parent() { return m_parent; }
    void set_parent(Function* parent) { m_parent = parent; }

    void clear_parent() { m_parent = nullptr; }

    const Type* get_allocated_type() const { return m_alloc_type; }

    u32 get_alignment() const { return m_align; }
    void set_alignment(u32 align) { m_align = align; }
};

} // namespace stm

#endif // STATIM_SIIR_LOCAL_HPP_
