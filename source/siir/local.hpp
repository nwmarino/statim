#ifndef STATIM_SIIR_LOCAL_HPP_
#define STATIM_SIIR_LOCAL_HPP_

#include "siir/value.hpp"

namespace stm {
namespace siir {

class CFG;
class Function;

/// The Local class represents named values living on the stack frame of a
/// function. They are the main targets of load/store operations in the IR.

/// A stack-based local variable in a function.
class Local final : public Value {
    /// The function that this local is in.
    Function* m_parent;

    /// The name of this local.
    std::string m_name;

    /// The type allocated for this local.
    const Type* m_alloc_type;

    /// The desired stack alignment of this local. 
    u32 m_align;

public:
    /// Create a new local, allocated for |type| with alignment |align|.
    Local(CFG& cfg, const Type* type, u32 align, const std::string& name, 
          Function* parent);

    /// Get the parent function this local is contained in.
    const Function* get_parent() const { return m_parent; }
    Function* get_parent() { return m_parent; }
    
    /// Clear the parent function link of this local. Does not remove this
    /// local from the old parent.
    void clear_parent() { m_parent = nullptr; }

    /// Mutate the parent function of this local to |parent|.
    void set_parent(Function* parent) { m_parent = parent; }

    /// Detaches this local from its parent function. Does not destroy the
    /// local.
    void detach_from_parent();

    /// Returns the name of this local.
    const std::string& get_name() const { return m_name; }

    /// Set the name of this local to |name|.
    void set_name(const std::string& name) { m_name = name; }

    /// Returns the type this local is allocated for.
    const Type* get_allocated_type() const { return m_alloc_type; }

    /// Get the desired alignment of this local. 
    u32 get_alignment() const { return m_align; }

    /// Mutate the desired alignment of this local to |align|.
    void set_alignment(u32 align) { m_align = align; }

    void print(std::ostream& os) const override;
};

} // namespace siir
} // namespace stm

#endif // STATIM_SIIR_LOCAL_HPP_
