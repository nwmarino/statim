//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LOVELACE_IR_VALUE_H_
#define LOVELACE_IR_VALUE_H_

//
//  This header file declares the Value class, which is the base for most of
//  the components that may make up the control-flow graph based IR.
//

#include "lir/graph/Type.hpp"

#include <cstdint>
#include <ostream>
#include <vector>

namespace lir {

class Use;
class User;

/// The different policies for printing values.
enum class PrintPolicy {
    Def,
    Use,
};

/// A typed value in the IR.
class Value {
public:
    using Uses = std::vector<Use*>;

protected:
    Type *m_type;

    /// The borrowed uses of this value. 
    Uses m_uses = {};
    
    Value(Type *type) : m_type(type) {}

public:
    virtual ~Value() = default;

    Value(const Value&) = delete;
    void operator=(const Value&) = delete;

    Value(Value&&) noexcept = delete;
    void operator=(Value&&) noexcept = delete;

    void set_type(Type *type) { m_type = type; }
    Type *get_type() const { return m_type; }

    const Uses &uses() const { return m_uses; }
    Uses &uses() { return m_uses; }

    /// Returns the first use of this value, if it exists.
    const Use *use_front() const { return m_uses.front(); }
    Use *use_front() { return m_uses.front(); }

    /// Returns the latest use of this value, if it exists.
    const Use *use_back() const { return m_uses.back(); }
    Use *use_back() { return m_uses.back(); }

    /// Returns the number of times this value is used.
    uint32_t num_uses() const { return m_uses.size(); }

    /// Returns true if this value has atleast one use.
    bool used() const { return !m_uses.empty(); }

    /// Returns true if this value has exactly one use.
    bool has_one_use() const { return m_uses.size() == 1; }

    /// Add |use| to the uses of this value.
    void add_use(Use *use) { m_uses.push_back(use); }

    /// Removes the edge |use| from this value, if it exists.
    void del_use(Use *use);

    /// Replace all uses of this value with the given |value|.
    void replace_all_uses_with(Value *value);

    /// Returns true if this value is a constant.
    virtual bool is_constant() const { return false; }

    /// Print this value in a reproducible plaintext format to |os|, following
    /// the given printing |policy|.
    virtual void print(std::ostream &os, PrintPolicy policy) const = 0;
};

} // namespace lir

#endif // LOVELACE_IR_VALUE_H_
