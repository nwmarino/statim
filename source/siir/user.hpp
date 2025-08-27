#ifndef STATIM_SIIR_USER_HPP_
#define STATIM_SIIR_USER_HPP_

#include "siir/use.hpp"
#include "siir/value.hpp"
#include <initializer_list>

namespace stm {

namespace siir {

/// Represents values in the intermediate representation that use other values.
class User : public Value {
protected:
    std::vector<Use> m_operands = {};

    User() = default;

    User(std::initializer_list<Value*> ops, const Type* type = nullptr, 
         const std::string& name = "")
        : Value(type, name) {
        for (auto v : ops)
            if (v) m_operands.emplace_back(v, this);
    }

public:
    const std::vector<Use>& operands() const { return m_operands; }

    const Use& get_operand(u32 i) const {
        assert(i <= m_operands.size());
        return m_operands[i];
    }
    
    Use& get_operand(u32 i) {
        assert(i <= m_operands.size());
        return m_operands[i];
    }

    u32 num_operands() const { return m_operands.size(); }
};

} // namespace siir

} // namespace stm

#endif // STATIM_SIIR_USER_HPP_
