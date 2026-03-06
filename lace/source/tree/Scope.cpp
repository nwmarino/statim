//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/tree/Defn.h"
#include "lace/tree/Scope.h"

using namespace lace;

Scope::~Scope() {
    for (Scope* scope : m_children) {
        if (scope)
            delete scope;
    }

    m_parent = nullptr;
    m_children.clear();
    m_symbols.clear();
}

bool Scope::add(const Symbol& symbol) {
    if (has(symbol.name))
        return false;

    m_symbols.emplace(symbol.name, symbol);
    return true;
}

bool Scope::has(const std::string& name) const {
    return m_symbols.contains(name);
}

bool Scope::get(const std::string& name, Symbol& symbol) const {
    auto it = m_symbols.find(name);
    if (it != m_symbols.end()) {
        symbol = it->second;
        return true;
    }

    if (has_parent())
        return m_parent->get(name, symbol);

    return false;
}
