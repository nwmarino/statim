#ifndef STATIM_SIIR_VALUE_HPP_
#define STATIM_SIIR_VALUE_HPP_

#include "core/type.hpp"

#include <string>
#include <vector>

namespace stm {

class Use;
class User;

/// Represents a value in the intermediate representation.
class Value {
protected:
    const Type* m_type;
    std::string m_name;
    std::vector<Use*> m_uses;

    Value() = default;
    Value(const Type* type, const std::string& name);

public:
    virtual ~Value() = default;

    const Type* get_type() const { return m_type; }
    void set_type(const Type* type) { m_type = type; }

    const std::string& get_name() const { return m_name; }
    void set_name(const std::string& name) { m_name = name; }

    bool has_name() const { return !m_name.empty(); }

    /// Get the uses of this value.
    const std::vector<Use*>& uses() const { return m_uses; }

    const Use* use_front() const { return m_uses.front(); }
    Use* use_front() { return m_uses.front(); }

    const Use* use_back() const { return m_uses.back(); }
    Use* use_back() { return m_uses.back(); }

    /// \returns The number of times this value is used.
    u32 num_uses() const { return m_uses.size(); }

    /// \returns `true` if this value is used.
    bool used() const { return m_uses.empty(); }

    /// \returns `true` if this value has exactly one use.
    bool has_one_use() const { return m_uses.size() == 1; }

    /// Adds a new \p use to this value.
    void add_use(Use* use);

    /// Removes a \p use from this value, if it is an existing edge.
    void del_use(Use* use);

    /// Replace all uses of this value with \p value.
    void replace_all_uses_with(Value* value);
};

} // namespace stm

#endif // STATIM_SIIR_VALUE_HPP_
