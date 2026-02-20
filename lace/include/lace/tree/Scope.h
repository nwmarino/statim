//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LACE_SCOPE_H_
#define LACE_SCOPE_H_

//
//  This header file defines the Scope class, which is a data structure created at parse time to 
//  organize which named definitions are visible at different points in the program. 
// 
//  The scope trees work in a lexical manner, i.e. depend on actual location in the source code,
//  and are used during syntax tree analysis passes.
//

#include "lace/core/Common.h"

#include <string>
#include <unordered_map>

namespace lace {

class NamedDefn;

class Scope final {
public:
    using DefnTable = std::unordered_map<std::string, NamedDefn*>;

private:
    Scope* m_parent;
    DefnTable m_defns = {};

public:
    Scope(Scope* parent = nullptr) : m_parent(parent) {}

    ~Scope() = default;

    Scope(const Scope&) = delete;
    void operator=(const Scope&) = delete;

    Scope(Scope&&) noexcept = delete;
    void operator=(Scope&&) noexcept = delete;

    /// Returns the scope which is the parent to this one, if it exists.
    Scope* getParent() const { return m_parent; }

    /// Test if this scope has a parent.
    Result hasParent() const { return m_parent != nullptr; }

    /// Add the given |defn| to this scope. 
    /// 
    /// If it conflicts name-wise with another definition, then the attempt returns false.
    Result add(NamedDefn* defn);

    /// Returns the definition in this scope with the given |name|, and null if one does not exist.
    NamedDefn* get(const std::string& name) const;
};

} // namespace lace

#endif // LACE_SCOPE_H_
