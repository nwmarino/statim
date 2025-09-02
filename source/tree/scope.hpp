#ifndef STATIM_SCOPE_HPP_
#define STATIM_SCOPE_HPP_

#include <string>
#include <unordered_map>

namespace stm {
     
class Decl;

/// A scope tree containing named symbols.
class Scope final {
    Scope* m_parent;
    std::unordered_map<std::string, Decl*> m_symbols {};

public:
    Scope(Scope* parent = nullptr) : m_parent(parent) {}

    /// Get the parent scope to this scope tree.
    const Scope* get_parent() const { return m_parent; }
    Scope* get_parent() { return m_parent; }

    const std::unordered_map<std::string, Decl*>& get_symbols() const { 
        return m_symbols; 
    }

    /// \returns The declaration in scope with name \p name, if it exists.
    Decl* get(const std::string& name) const;

    /// Attempt to add \p decl to this scope.
    /// \returns `false` if the declaration has conflicts.
    bool add(Decl* decl);
};

} // namespace stm

#endif // STATIM_SCOPE_HPP_
