#ifndef STATIM_SIIR_USER_HPP_
#define STATIM_SIIR_USER_HPP_

#include "siir/use.hpp"
#include "siir/value.hpp"

namespace stm {
namespace siir {

/// A value that uses other values.
class User : public Value {
protected:
    /// The operands of this user, or "use" edges - value, user pairs that
    /// model the use-def chain.
    std::vector<Use*> m_operands = {};

    User() = default;
    User(const std::vector<Value*>& ops, const Type* type) : Value(type) {
        for (auto& v : ops) {
            if (v)
                m_operands.emplace_back(new Use(v, this));
        }
    }

public:
    ~User() {
        for (auto& use : m_operands) delete use;
        m_operands.clear();
    }

    /// Get the operand list of this user.
    const std::vector<Use*>& get_operand_list() const { return m_operands; }
    std::vector<Use*>& get_operand_list() { return m_operands; }

    /// Get the operand at position |i| of this user.
    const Use* get_operand(u32 i) const {
        assert(i <= num_operands());
        return m_operands[i];
    }
    
    Use* get_operand(u32 i) {
        assert(i <= num_operands());
        return m_operands[i];
    }

    /// Returns the number of operands this user has.
    u32 num_operands() const { return m_operands.size(); }

    /// Add a new operand |op| to this user.
    void add_operand(Value* value) {
        m_operands.push_back(new Use(value, this));
    }
};

} // namespace siir
} // namespace stm

#endif // STATIM_SIIR_USER_HPP_
