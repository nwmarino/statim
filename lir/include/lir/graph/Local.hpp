//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LOVELACE_IR_LOCAL_H_
#define LOVELACE_IR_LOCAL_H_

#include "lir/graph/Value.hpp"

#include <cstdint>

namespace lir {

class CFG;
class Function;

/// A local variable that sits on the stack frame of a function in the IR.
class Local final : public Value {
    Function *m_parent;
    std::string m_name; 
    uint32_t m_align;

    Local(PointerType *type, Function *parent, const std::string &name, 
          uint32_t align)
      : Value(type), m_parent(parent), m_name(name), m_align(align) {}

public:
    [[nodiscard]] static
    Local *create(CFG &cfg, Type *type, const std::string &name, 
                  Function *parent = nullptr, uint32_t align = 0);

    void set_parent(Function *parent) { m_parent = parent; }
    const Function *get_parent() const { return m_parent; }
    Function *get_parent() { return m_parent; }

    /// Test if this local has a parent function.
    bool has_parent() const { return m_parent != nullptr; }

    /// Detaches this local from its parent function. 
    /// Does not free any memory allocated for this local.
    void detach();

    void set_name(const std::string &name) { m_name = name; }
    const std::string &get_name() const { return m_name; }
    std::string &get_name() { return m_name; }

    /// Returns the type that this local is allocated for.
    const Type *get_allocated_type() const { 
        return static_cast<PointerType*>(m_type)->get_pointee(); 
    }

    Type *get_allocated_type() { 
        return static_cast<PointerType*>(m_type)->get_pointee();
    }

    void set_align(uint32_t align) { m_align = align; }
    uint32_t get_align() const { return m_align; }

    void print(std::ostream &os, PrintPolicy policy) const override;
};

} // namespace lir

#endif // LOVELACE_IR_LOCAL_H_
