//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/core/Context.h"
#include "lace/tree/Rib.h"

using namespace lace;

Context::~Context() {
    for (auto& [name, rib] : m_ribs) {
        if (rib)
            delete rib;
    }

    m_ribs.clear();
}

bool Context::add_rib(Rib* rib) {
    if (get_rib(rib->name()) != nullptr)
        return false;
    
    m_ribs.emplace(rib->name(), rib);
    return true;
}

Rib* Context::get_rib(const std::string& name) const {
    assert(!name.empty() && "rib name cannot be empty!");

    auto it = m_ribs.find(name);
    if (it != m_ribs.end())
        return it->second;

    return nullptr;
}
