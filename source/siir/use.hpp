#ifndef STATIM_SIIR_USE_HPP_
#define STATIM_SIIR_USE_HPP_

#include "value.hpp"

namespace stm {
    
namespace siir {

class User;

/// Represents a use; the edge between a value and a user of it.
class Use final {
    Value* m_value;
    User* m_user;

public:
    Use(Value* value, User* user) : m_value(value), m_user(user) {
        if (value)
            value->add_use(this);
    }

    const Value* get_value() const { return m_value; }
    Value* get_value() { return m_value; }

    const User* get_user() const { return m_user; }
    User* get_user() { return m_user; }

    void set_value(Value* value) { m_value = value; }
    void set_user(User* user) { m_user = user; }
};

} // namespace siir

} // namespace stm

#endif // STATIM_SIIR_USE_HPP_
