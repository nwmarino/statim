//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LOVELACE_IR_PARAMETER_H_
#define LOVELACE_IR_PARAMETER_H_

#include "lir/graph/Value.h"

namespace lir {

using Result = bool;

class Function;

/// A parameter to a function in the IR.
class Parameter final : public Value {
public:
    enum class Trait : uint32_t {
        None = 0,
        ARet,
        Byval,  
    };

private:
    Function *m_parent;
    std::string m_name;
    Trait m_trait;

    Parameter(Type *type, Function *parent, const std::string &name, Trait trait)
      : Value(type), m_parent(parent), m_name(name), m_trait(trait) {}

public:
    [[nodiscard]] static 
    Parameter *create(Type *type, const std::string &name = "", Trait trait = Trait::None,
                      Function *parent = nullptr);

    ~Parameter() = default;

    Parameter(const Parameter&) = delete;
    void operator=(const Parameter&) = delete;

    Parameter(Parameter&&) noexcept = delete;
    void operator=(Parameter&&) noexcept = delete;

    void set_parent(Function *function) { m_parent = function; }
    const Function *get_parent() const { return m_parent; }
    Function *get_parent() { return m_parent; }
    
    /// Test if this parameter has a parent function.
    bool has_parent() const { return m_parent != nullptr; }

    void set_name(const std::string &name) { m_name = name; }
    const std::string &get_name() const { return m_name; }
    std::string &get_name() { return m_name; }

    /// Test if this argument is named.
    bool is_named() const { return !m_name.empty(); }

    /// Set the trait of this parameter to |trait|.
    void setTrait(Trait trait) { m_trait = trait; }

    /// Returns the trait of this parameter, if it has one.
    Trait getTrait() const { return m_trait; }

    /// Test if this parameter has a trait.
    Result hasTrait() const { return m_trait != Trait::None; }

    /// Test if this parameter has the given |trait|.
    Result hasTrait(Trait trait) const { return m_trait == trait; }

    /// Returns the index of this argument in its parent function.
    ///
    /// If this argument is not attached to a function, then this function 
    /// fails by assertion.
    uint32_t get_index() const;

    void print(std::ostream &os, PrintPolicy policy) const override;
};

} // namespace lir

#endif // LOVELACE_IR_PARAMETER_H_
