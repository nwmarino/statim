//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/tree/Defn.h"
#include "lace/tree/Expr.h"
#include "lace/tree/Type.h"

using namespace lace;

//>==---------------------------------------------------------------------------
//                          UseDefn Implementation
//>==---------------------------------------------------------------------------

UseDefn* UseDefn::create(Rib& rib, SourceSpan span, const std::string& path) {
    return new UseDefn(&rib, span, path);
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
//                          VariableDefn Implementation
//>==---------------------------------------------------------------------------

VariableDefn* VariableDefn::create(Rib& rib, SourceSpan span, 
                                   const std::string& name, const Runes& runes, 
                                   Type* type, Expr* init, bool global) {
    return new VariableDefn(&rib, span, name, runes, type, init, global);
}

VariableDefn::~VariableDefn() {
    if (m_init)
        delete m_init;

    m_init = nullptr;
}

//>==---------------------------------------------------------------------------
//                          ParameterDefn Implementation
//>==---------------------------------------------------------------------------

ParameterDefn* ParameterDefn::create(Rib& rib, SourceSpan span, 
                                     const std::string& name, 
                                     const Runes& runes, Type* type) {
    return new ParameterDefn(&rib, span, name, runes, type);
}

//>==---------------------------------------------------------------------------
//                          FunctionDefn Implementation
//>==---------------------------------------------------------------------------

FunctionDefn* FunctionDefn::create(Rib& rib, SourceSpan span, 
                                   const std::string& name, const Runes& runes, 
                                   FunctionType* type, Scope* scope, 
                                   ParameterDefn* receiver, const Params& params, 
                                   BlockStmt* body) {
    return new FunctionDefn(
        &rib, 
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

FieldDefn* FieldDefn::create(Rib& rib, SourceSpan span, 
                             const std::string& name, const Runes& runes, 
                             Type* type, uint32_t index) {
    return new FieldDefn(&rib, span, name, runes, type, index);
}

//>==---------------------------------------------------------------------------
//                          VariantDefn Implementation
//>==---------------------------------------------------------------------------

VariantDefn* VariantDefn::create(Rib& rib, SourceSpan span, 
                                 const std::string& name, const Runes& runes, 
                                 Type* type, int64_t value) {
    return new VariantDefn(&rib, span, name, runes, type, value);
}

//>==---------------------------------------------------------------------------
//                          AliasDefn Implementation
//>==---------------------------------------------------------------------------

AliasDefn* AliasDefn::create(Rib& rib, SourceSpan span, 
                             const std::string& name, const Runes& runes, 
                             Type* type) {
    return new AliasDefn(&rib, span, name, runes, type);
}

//>==---------------------------------------------------------------------------
//                          StructDefn Implementation
//>==---------------------------------------------------------------------------

StructDefn* StructDefn::create(Rib& rib, SourceSpan span, 
                               const std::string& name, const Runes& runes, 
                               Type* type) {
    return new StructDefn(&rib, span, name, runes, type);
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

EnumDefn* EnumDefn::create(Rib& rib, SourceSpan span, 
                           const std::string& name, const Runes& runes, 
                           Type* type) {
    return new EnumDefn(&rib, span, name, runes, type);
}

EnumDefn::~EnumDefn() {
    for (VariantDefn* variant : m_variants)
        delete variant;

    m_variants.clear();
}
