//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LACE_SCOPE_H_
#define LACE_SCOPE_H_

//
//  This header file defines the Scope class, which is a data structure created 
//  at parse time to organize which named definitions are visible at different 
//  points in the program. 
// 
//  The scope trees work in a lexical manner, i.e. depend on actual location in 
//  the source code, and are used during analysis passes.
//

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace lace {

class Defn;
class NamedDefn;
class SpaceDefn;
class Type;

struct Symbol final {
    enum class Kind : uint32_t {
        Definition,
        Type,
    };

    enum class Visibility : uint32_t {
        Public,
        Private,
    };

    std::string name;
    std::string qual_name;
    Kind kind;
    Visibility visibility;
    union {
        NamedDefn* defn;
        Type* type;
    };
};

using SymbolTable = std::unordered_map<std::string, Symbol>;

class Scope final {
    Scope* m_parent;
    std::vector<Scope*> m_children = {};
    SymbolTable m_symbols = {};

public:
    Scope(Scope* parent = nullptr) : m_parent(parent) {}

    ~Scope();

    Scope(const Scope&) = delete;
    void operator=(const Scope&) = delete;

    Scope(Scope&&) noexcept = delete;
    void operator=(Scope&&) noexcept = delete;

    /// Returns the scope tree which is the parent to this one, if it exists, 
    /// and null otherwise.
    Scope* parent() const { return m_parent; }

    /// Test if this scope tree has a parent.
    bool has_parent() const { return m_parent != nullptr; }

    /// Returns the child scopes of this scope.
    const std::vector<Scope*>& children() const { return m_children; }
    std::vector<Scope*>& children() { return m_children; }

    /// Returns the number of child scopes this scope has.
    uint32_t num_children() const { return m_children.size(); }

    /// Test if this scope has any child nodes.
    bool has_children() const { return !m_children.empty(); }

    /// Returns the symbol table of this scope.
    const SymbolTable& symbols() const { return m_symbols; }
    SymbolTable& symbols() { return m_symbols; }

    /// Add the given |symbol| to this scope. 
    /// If it conflicts name-wise with another symbol, then the attempt 
    /// returns false.
    [[nodiscard]] bool add(const Symbol& symbol);

    /// Test if this scope contains a symbol with the given |name|.
    [[nodiscard]] bool has(const std::string& name) const;

    /// Dispatch the symbol in this scope with the given |name| to |symbol|.
    /// Returns the result of whether the symbol could be found.
    [[nodiscard]] bool get(const std::string& name, Symbol& symbol) const;
};

} // namespace lace

#endif // LACE_SCOPE_H_
