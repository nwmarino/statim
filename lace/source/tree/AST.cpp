//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/tree/AST.h"
#include "lace/tree/Defn.h"
#include "lace/tree/Type.h"

using namespace lace;

AST::AST(const std::string& file) : m_file(file) {
    m_scope = new Scope();

    // Initialize all built-in types.
    for (uint32_t i = static_cast<uint32_t>(BuiltinType::Kind::Void); i <= static_cast<uint32_t>(BuiltinType::Kind::Float64); ++i) {
        BuiltinType::Kind kind = static_cast<BuiltinType::Kind>(i);
        m_types.builtins.push_back(new BuiltinType(kind));
    }
}

AST::~AST() {
    for (auto& [name, type] : m_types.aliases) {
        if (type)
            delete type;
    }

    for (auto& type : m_types.builtins) {
        if (type)
            delete type;
    }

    for (auto& type : m_types.deferred) {
        if (type)
            delete type;
    }

    for (auto& [name, type] : m_types.enums) {
        if (type)
            delete type;
    }

    for (auto& type : m_types.functions) {
        if (type)
            delete type;
    }

    for (auto& type : m_types.pointers) {
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

AST* AST::create(const std::string& file) {
    return new AST(file);
}
