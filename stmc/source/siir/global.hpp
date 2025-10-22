#ifndef STATIM_SIIR_GLOBAL_HPP_
#define STATIM_SIIR_GLOBAL_HPP_

#include "siir/constant.hpp"

namespace stm {
namespace siir {

class CFG;

/// A top-level global variable possibly initialized with a constant.
class Global final : public Constant {
public:
    /// Recognized linkage types for global data.
    enum LinkageType : u8 {
        LINKAGE_INTERNAL, 
        LINKAGE_EXTERNAL,
    };

private:
    /// The parent graph of this function.
    CFG* m_parent;

    /// The name of this global variable.
    std::string m_name;

    /// The optional, constant initializer of this data.
    Constant* m_init;

    /// The linkage type of this named data.
    LinkageType m_linkage;

    /// If true, this data is read only which means it cannot be mutated and
    /// has some special lowering requirements.
    bool m_read_only;

public:
    /// Create a new global of the given |type|. Use |read_only| to assert that
    /// no mutations can occur to the new global. If |read_only| is true, then
    /// the |init| argument must also be provided.
    Global(CFG& cfg, const Type* type, LinkageType linkage, bool read_only, 
           const std::string& name, Constant* init = nullptr);

    Global(const Global&) = delete;
    Global& operator = (const Global&) = delete;

    /// Returns the parent graph of this global.
    const CFG* get_parent() const { return m_parent; }
    CFG* get_parent() { return m_parent; }

    /// Clear the parent graph link of this global. Does not remove it from
    /// the old graph.
    void clear_parent() { m_parent = nullptr; }

    /// Mutate the parent graph of this global to |parent|. Does not add this
    /// global to the new parent, nor does it remove it from the old one.
    void set_parent(CFG* parent) { m_parent = parent; }

    /// Get the name of this global variable.
    const std::string& get_name() const { return m_name; }

    /// Set the name of this global to variable to |name|.
    void set_name(const std::string& name) { m_name = name; }

    /// Returns the constant initializer of this data, if it exists.
    Constant* get_initializer() const { return m_init; }

    /// Set the initializer of this global data to |constant|.
    void set_initializer(Constant* constant) { m_init = constant; }

    /// Returns true if this global has a constant initializer.
    bool has_initializer() const { return m_init != nullptr; }

    /// Returns the type of linkage used for this global variable.
    LinkageType get_linkage() const { return m_linkage; }

    /// Mutate the type of linkage this global has to |linkage|.
    void set_linkage(LinkageType linkage) { m_linkage = linkage; }
    
    /// Returns true if this global is marked as read-only.
    bool is_read_only() const { return m_read_only; }

    /// Set the read-only flag of this global to |value|.
    void set_read_only(bool value = true) { m_read_only = value; }

    void print(std::ostream& os) const override;
};

} // namespace siir
} // namespace stm

#endif // STATIM_SIIR_GLOBAL_HPP_
