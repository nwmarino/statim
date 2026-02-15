//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/tree/Defn.h"
#include "lace/tree/Scope.h"

using namespace lace;

bool Scope::add(NamedDefn* defn) {
    if (get(defn->get_name()))
        return false;

    m_defns.emplace(defn->get_name(), defn);
    return true;
}

NamedDefn* Scope::get(const std::string& name) const {
    auto it = m_defns.find(name);
    if (it != m_defns.end())
        return it->second;

    if (m_parent)
        return m_parent->get(name);

    return nullptr;
}
