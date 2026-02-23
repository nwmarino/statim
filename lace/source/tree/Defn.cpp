//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/tree/Defn.h"
#include "lace/tree/Expr.h"
#include "lace/tree/Type.h"

using namespace lace;

//>==---------------------------------------------------------------------------
//                          LoadDefn Implementation
//>==---------------------------------------------------------------------------

LoadDefn* LoadDefn::create(AST& ast, SourceSpan span, 
                           const std::string& path) {
    return new LoadDefn(&ast, span, path);
}

//>==---------------------------------------------------------------------------
//                          NamedDefn Implementation
//>==---------------------------------------------------------------------------

NamedDefn::~NamedDefn() {
    for (Rune* rune : m_runes) {
        if (rune)
            delete rune;
    }

    m_runes.clear();
}

//>==---------------------------------------------------------------------------
//                          SpaceDefn Implementation
//>==---------------------------------------------------------------------------

SpaceDefn* SpaceDefn::create(AST& ast, SourceSpan span, const std::string& name,
                             const Runes& runes, Scope* scope,
                             const std::vector<NamedDefn*>& defns) {
    return new SpaceDefn(&ast, span, name, runes, scope, defns);
}

SpaceDefn::~SpaceDefn() {
    //if (m_scope)
    //    delete m_scope;

    m_scope = nullptr;

    for (NamedDefn* defn : m_defns) {
        // Only delete definitions defined in the same file.
        if (defn && defn->origin() == m_origin)
            delete defn;
    }

    m_defns.clear();
}

//>==---------------------------------------------------------------------------
//                          VariableDefn Implementation
//>==---------------------------------------------------------------------------

VariableDefn* VariableDefn::create(AST& ast, SourceSpan span, 
                                   const std::string& name, const Runes& runes, 
                                   Type* type, Expr* init, bool global) {
    return new VariableDefn(&ast, span, name, runes, type, init, global);
}

VariableDefn::~VariableDefn() {
    if (m_init)
        delete m_init;
        
    m_init = nullptr;
}

//>==---------------------------------------------------------------------------
//                          ParameterDefn Implementation
//>==---------------------------------------------------------------------------

ParameterDefn* ParameterDefn::create(AST& ast, SourceSpan span, 
                                     const std::string& name, 
                                     const Runes& runes, Type* type) {
    return new ParameterDefn(&ast, span, name, runes, type);
}

//>==---------------------------------------------------------------------------
//                          FunctionDefn Implementation
//>==---------------------------------------------------------------------------

FunctionDefn* FunctionDefn::create(AST& ast, SourceSpan span, 
                                   const std::string& name, const Runes& runes, 
                                   FunctionType* type, Scope* scope, 
                                   ParameterDefn* receiver, const Params& params, 
                                   BlockStmt* body) {
    return new FunctionDefn(
        &ast, 
        span, 
        name, 
        runes, 
        type, 
        scope, 
        receiver, 
        params,
        body
    );
}

FunctionDefn::~FunctionDefn() {
    //if (m_scope)
    //    delete m_scope;
    
    m_scope = nullptr;

    for (ParameterDefn* param : m_params) {
        if (param)    
            delete param;
    }

    m_params.clear();

    if (m_body)
        delete m_body;
        
    m_body = nullptr;
}

const Type* FunctionDefn::get_receiver_type() const {
    if (!has_receiver())
        return nullptr;

    Type* type = m_receiver->type();
    assert(type);

    auto ptr = dynamic_cast<PointerType*>(type);
    assert(ptr);

    return ptr->pointee();
}

//>==---------------------------------------------------------------------------
//                          FieldDefn Implementation
//>==---------------------------------------------------------------------------

FieldDefn* FieldDefn::create(AST& ast, SourceSpan span, 
                             const std::string& name, const Runes& runes, 
                             Type* type, uint32_t index) {
    return new FieldDefn(&ast, span, name, runes, type, index);
}

//>==---------------------------------------------------------------------------
//                          VariantDefn Implementation
//>==---------------------------------------------------------------------------

VariantDefn* VariantDefn::create(AST& ast, SourceSpan span, 
                                 const std::string& name, const Runes& runes, 
                                 Type* type, int64_t value) {
    return new VariantDefn(&ast, span, name, runes, type, value);
}

//>==---------------------------------------------------------------------------
//                          AliasDefn Implementation
//>==---------------------------------------------------------------------------

AliasDefn* AliasDefn::create(AST& ast, SourceSpan span, 
                             const std::string& name, const Runes& runes, 
                             Type* type) {
    return new AliasDefn(&ast, span, name, runes, type);
}

//>==---------------------------------------------------------------------------
//                          StructDefn Implementation
//>==---------------------------------------------------------------------------

StructDefn* StructDefn::create(AST& ast, SourceSpan span, 
                               const std::string& name, const Runes& runes, 
                               Type* type) {
    return new StructDefn(&ast, span, name, runes, type);
}

StructDefn::~StructDefn() {
    for (FieldDefn* field : m_fields) {
        if (field)
            delete field;
    }

    m_fields.clear();
    m_methods.clear();
}

//>==---------------------------------------------------------------------------
//                          EnumDefn Implementation
//>==---------------------------------------------------------------------------

EnumDefn* EnumDefn::create(AST& ast, SourceSpan span, 
                           const std::string& name, const Runes& runes, 
                           Type* type) {
    return new EnumDefn(&ast, span, name, runes, type);
}

EnumDefn::~EnumDefn() {
    for (VariantDefn* variant : m_variants)
        delete variant;

    m_variants.clear();
}
