//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/tree/AST.h"
#include "lace/tree/Defn.h"
#include "lace/tree/Type.h"

using namespace lace;

AST::Context::Context() {
    // Initialize all built-in types.
    for (uint32_t i = static_cast<uint32_t>(BuiltinType::Kind::Void); i <= static_cast<uint32_t>(BuiltinType::Kind::Float64); ++i) {
        BuiltinType::Kind kind = static_cast<BuiltinType::Kind>(i);
        m_builtins.push_back(new BuiltinType(kind));
    }
}

AST::Context::~Context() {
    for (auto& [name, type] : m_aliases)
        delete type;

    for (auto& type : m_arrays)
        delete type;

    for (auto& type : m_builtins)
        delete type;

    for (auto& type : m_deferred)
        delete type;

    for (auto& [name, type] : m_enums)
        delete type;

    for (auto& type : m_functions)
        delete type;

    for (auto& type : m_pointers)
        delete type;

    for (auto& [name, type] : m_structs)
        delete type;

    m_aliases.clear();
    m_arrays.clear();
    m_builtins.clear();
    m_deferred.clear();
    m_enums.clear();
    m_functions.clear();
    m_pointers.clear();
    m_structs.clear();
}

AST::AST(const std::string& file) : m_file(file) {
    m_scope = new Scope();
}

AST::~AST() {
    delete m_scope;
    m_scope = nullptr;

    for (Defn* defn : m_defns)
        delete defn;

    m_defns.clear();
}

AST* AST::create(const std::string& file) {
    return new AST(file);
}
