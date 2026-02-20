//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lir/machine/MachineFunction.hpp"
#include "lir/machine/MachineObject.hpp"

using namespace lir;

MachineObject::~MachineObject() {
    for (auto &[name, global] : m_globals)
        delete global;

    for (auto &[name, func] : m_functions)
        delete func;
}

const MachineData *MachineObject::get_global(const std::string &name) const {
    auto it = m_globals.find(name);
    if (it != m_globals.end())
        return it->second;

    return nullptr;
}

const MachineFunction *MachineObject::get_function(const std::string &name) const {
    auto it = m_functions.find(name);
    if (it != m_functions.end())
        return it->second;

    return nullptr;
}
