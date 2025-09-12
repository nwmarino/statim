#ifndef STATIM_SIIR_INLINE_ASM_H_
#define STATIM_SIIR_INLINE_ASM_H_

#include "siir/value.hpp"

namespace stm::siir {

class InlineAsm final : public Value {
    std::string m_iasm;
    std::vector<std::string> m_constraints;
    bool m_side_effects;

public:
    InlineAsm(const FunctionType* type, const std::string& iasm, 
              const std::vector<std::string>& constraints, bool side_effects)
        : Value(type), m_iasm(iasm), m_constraints(constraints), 
          m_side_effects(side_effects) {}

    InlineAsm(const InlineAsm&) = delete;
    InlineAsm& operator = (const InlineAsm&) = delete;

    /// Return the assembly string for this inline asm.
    const std::string& string() const { return m_iasm; }

    /// Returns the list of constraints for this inline asm.
    const std::vector<std::string>& constraints() const { 
        return m_constraints; 
    }

    /// Returns true if this inline asm is said to have side effects.
    bool has_side_effects() const { return m_side_effects; }

    void print(std::ostream& os) const override;
};

} // namespace stm::siir

#endif // STATIM_SIIR_INLINE_ASM_H_
