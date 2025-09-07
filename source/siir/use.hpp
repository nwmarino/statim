#ifndef STATIM_SIIR_USE_HPP_
#define STATIM_SIIR_USE_HPP_

#include "value.hpp"

namespace stm {
namespace siir {

class User;

/// Represents a use; the edge between a value and a user of it.
class Use final {
    /// The value being used.
    Value* m_value;

    /// The value/user that is using the value in the edge.
    User* m_user;

public:
    /// Create a new use edge between a value and a user.
    Use(Value* value, User* user);

    ~Use();

    operator Value*() { return m_value; }
    operator const Value*() { return m_value; }

    operator User*() { return m_user; }
    operator const User*() const { return m_user; }

    /// Get the value of this use.
    const Value* get_value() const { return m_value; }
    Value* get_value() { return m_value; }

    /// Set the value of this use to |value|.
    void set_value(Value* value) {
        assert(m_value);
        assert(value);

        if (m_value == value) 
            return;

        m_value->del_use(this);
        m_value = value;
        m_value->add_use(this);
    }

    /// Get the user of this use.
    const User* get_user() const { return m_user; }
    User* get_user() { return m_user; }
};

} // namespace siir
} // namespace stm

#endif // STATIM_SIIR_USE_HPP_
