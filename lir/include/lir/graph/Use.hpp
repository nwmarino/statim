//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LOVELACE_IR_USE_H_
#define LOVELACE_IR_USE_H_

//
//  This header file declares the Use class, which models a def-use edge in
//  the IR.
//

#include "lir/graph/Value.hpp"

namespace lir {

class User;

/// Represents a use; the edge between a value and a user thereof.
class Use final {
    /// The value being used.
    Value *m_value;

    /// The value/user that is using the value in the edge.
    User *m_user;

public:
    Use(Value *value, User *user) : m_value(value), m_user(user) { 
        value->add_use(this); 
    }

    ~Use() {
        if (m_value)
            m_value->del_use(this);
        
        m_value = nullptr;
        m_user = nullptr;
    }

    Use(const Use&) = delete;
    void operator=(const Use&) = delete;

    Use(Use&&) noexcept = delete;
    void operator=(Use&&) noexcept = delete;

    operator Value*() { return m_value; }
    operator User*() { return m_user; }

    const Value *get_value() const { return m_value; }
    Value *get_value() { return m_value; }

    /// Set the value of this use to |value|.
    void set_value(Value *value) {
        assert(m_value && "current use value is null!");
        assert(value && "new use value cannot be null!");

        if (m_value != value) {
            m_value->del_use(this);
            m_value = value;
            m_value->add_use(this);
        }
    }

    const User *get_user() const { return m_user; }
    User *get_user() { return m_user; }
};

} // namespace lir

#endif // LOVELACE_IR_USE_H_
