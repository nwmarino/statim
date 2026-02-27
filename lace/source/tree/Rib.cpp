//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/tree/Rib.h"
#include "lace/tree/Defn.h"
#include "lace/tree/Type.h"

using namespace lace;

Rib::Rib(const std::string& name, const std::string& path) 
  : m_name(name), m_path(path) {
    m_scope = new Scope();

    // Initialize all built-in types.
    uint32_t i = static_cast<uint32_t>(BuiltinType::Kind::Void);
    while (i <= static_cast<uint32_t>(BuiltinType::Kind::Float64)) {
        BuiltinType::Kind kind = static_cast<BuiltinType::Kind>(i);
        m_types.builtins.push_back(new BuiltinType(kind));
        i++;
    }
}

Rib::~Rib() {
    for (auto& [name, type] : m_types.aliases) {
        if (type)
            delete type;
    }

    for (BuiltinType* type : m_types.builtins) {
        if (type)
            delete type;
    }

    for (DeferredType* type : m_types.deferred) {
        if (type)
            delete type;
    }

    for (auto& [name, type] : m_types.enums) {
        if (type)
            delete type;
    }

    for (FunctionType* type : m_types.functions) {
        if (type)
            delete type;
    }

    for (PointerType* type : m_types.pointers) {
        if (type)
            delete type;
    }

    for (auto& [name, type] : m_types.structs) {
        if (type)
            delete type;
    }

    m_types.aliases.clear();
    m_types.builtins.clear();
    m_types.deferred.clear();
    m_types.enums.clear();
    m_types.functions.clear();
    m_types.pointers.clear();
    m_types.structs.clear();

    if (m_scope)
        delete m_scope;

    m_scope = nullptr;

    for (Defn* defn : m_defns) {
        if (defn)
            delete defn;
    }

    m_defns.clear();
}

Rib* Rib::create(const std::string& name, const std::string& path) {
    return new Rib(name, path);
}
