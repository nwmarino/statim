//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LOVELACE_IR_GLOBAL_H_
#define LOVELACE_IR_GLOBAL_H_

#include "lir/graph/Constant.hpp"

#include <cstdint>

namespace lir {

class CFG;

/// A top-level global variable possibly initialized with a constant.
class Global final : public Constant {
public:
    /// The different kinds of linkage a global can have.
    enum class LinkageType : uint8_t {
        Public,
        Private,
    };

private:
    CFG *m_parent;
    LinkageType m_linkage;
    std::string m_name;
    bool m_mut;

    Global(Type *type, CFG *parent, LinkageType linkage, 
           const std::string &name, bool mut, Constant *init)
      : Constant(type, { init }), m_parent(parent), m_linkage(linkage), 
        m_name(name), m_mut(mut) {}

public:
    [[nodiscard]] static 
    Global *create(CFG &cfg, Type *type, LinkageType linkage, 
                   const std::string &name, bool mut, Constant *init = nullptr);

    void set_parent(CFG *parent) { m_parent = parent; }
    const CFG *get_parent() const { return m_parent; }
    CFG *get_parent() { return m_parent; }

    /// Test if this global belongs to a graph.
    bool has_parent() const { return m_parent != nullptr; }

    void set_linkage(LinkageType linkage) { m_linkage = linkage; }
    LinkageType get_linkage() const { return m_linkage; }

    /// Test if this global has the given |linkage| type.
    bool has_linkage(LinkageType linkage) const { 
        return m_linkage == linkage; 
    }

    void set_mutable(bool mut = true) { m_mut = mut; }
    bool is_mutable() const { return m_mut; }

    void set_name(const std::string &name) { m_name = name; }
    const std::string &get_name() const { return m_name; }

    void set_initializer(Constant *init) {
        assert(!has_initializer() && "global already has an initializer!");
        add_operand(init); 
    }

    Constant* get_initializer() const { 
        return has_initializer() 
            ? static_cast<Constant*>(m_operands[0]->get_value()) 
            : nullptr;
    }

    /// Test if this global has a constant initializer.
    bool has_initializer() const { return has_operands(); }

    void print(std::ostream &os, PrintPolicy policy) const override;
};

} // namespace lir

#endif // LOVELACE_IR_GLOBAL_H_
