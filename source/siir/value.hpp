#ifndef STATIM_SIIR_VALUE_HPP_
#define STATIM_SIIR_VALUE_HPP_

#include "siir/type.hpp"

#include <ostream>
#include <vector>

namespace stm {
namespace siir {

class Use;
class User;

/// A value in the intermediate representation.
class Value {
protected:
    const Type* m_type;
    std::vector<Use*> m_uses = {};

    Value() = default;
    Value(const Type* type) : m_type(type) {}

public:
    virtual ~Value() = default;

    /// Returns the type of this value.
    const Type* get_type() const { return m_type; }

    /// Set the type of this value to |type|.
    void set_type(const Type* type) { m_type = type; }

    /// Returns true if this value has a type.
    bool has_type() const { return m_type != nullptr; }

    /// Get all uses of this value.
    const std::vector<Use*>& uses() const { return m_uses; }
    std::vector<Use*>& uses() { return m_uses; }

    /// Returns the first use of this value, if it exists.
    const Use* use_front() const { return m_uses.front(); }
    Use* use_front() { return m_uses.front(); }

    /// Returns the latest use of this value, if it exists.
    const Use* use_back() const { return m_uses.back(); }
    Use* use_back() { return m_uses.back(); }

    /// Returns the number of times this value is used.
    u32 num_uses() const { return m_uses.size(); }

    /// Returns true if this value has atleast one use.
    bool used() const { return !m_uses.empty(); }

    /// Returns true if this value has exactly one use.
    bool has_one_use() const { return m_uses.size() == 1; }

    /// Add |use| to the uses of this value.
    void add_use(Use* use);

    /// Removes the edge |use| from this value, if it exists.
    void del_use(Use* use);

    /// Replace all uses of this value with |value|.
    void replace_all_uses_with(Value* value);

    /// Returns true if this value is a constant.
    virtual bool is_constant() const { return false; }

    /// Print this value in a reproducible plaintext format to the output
    /// stream |os|.
    virtual void print(std::ostream& os) const = 0;
};

} // namespace siir
} // namespace stm

#endif // STATIM_SIIR_VALUE_HPP_
