//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/tree/Defn.h"
#include "lace/tree/Expr.h"

using namespace lace;

//>==---------------------------------------------------------------------------
//                          LoadDefn Implementation
//>==---------------------------------------------------------------------------

LoadDefn* LoadDefn::create(AST::Context& ctx, SourceSpan span, 
                           const std::string& path) {
    return new LoadDefn(span, path);
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

VariableDefn* VariableDefn::create(AST::Context& ctx, SourceSpan span, 
                                   const std::string& name, const Runes& runes, 
                                   Type* type, Expr* init, bool global) {
    return new VariableDefn(span, name, runes, type, init, global);
}

VariableDefn::~VariableDefn() {
    if (m_init)
        delete m_init;
        
    m_init = nullptr;
}

//>==---------------------------------------------------------------------------
//                          ParameterDefn Implementation
//>==---------------------------------------------------------------------------

ParameterDefn* ParameterDefn::create(AST::Context& ctx, SourceSpan span, 
                                     const std::string& name, 
                                     const Runes& runes, Type* type) {
    return new ParameterDefn(span, name, runes, type);
}

//>==---------------------------------------------------------------------------
//                          FunctionDefn Implementation
//>==---------------------------------------------------------------------------

FunctionDefn* FunctionDefn::create(AST::Context& ctx, SourceSpan span, 
                                   const std::string& name, const Runes& runes, 
                                   FunctionType* type, Scope* scope, 
                                   const Params& params, BlockStmt* body) {
    return new FunctionDefn(span, name, runes, type, scope, params, body);
}

FunctionDefn::~FunctionDefn() {
    if (m_scope)
        delete m_scope;
    
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

//>==---------------------------------------------------------------------------
//                          FieldDefn Implementation
//>==---------------------------------------------------------------------------

FieldDefn* FieldDefn::create(AST::Context& ctx, SourceSpan span, 
                             const std::string& name, const Runes& runes, 
                             Type* type, uint32_t index) {
    return new FieldDefn(span, name, runes, type, index);
}

//>==---------------------------------------------------------------------------
//                          VariantDefn Implementation
//>==---------------------------------------------------------------------------

VariantDefn* VariantDefn::create(AST::Context& ctx, SourceSpan span, 
                                 const std::string& name, const Runes& runes, 
                                 Type* type, int64_t value) {
    return new VariantDefn(span, name, runes, type, value);
}

//>==---------------------------------------------------------------------------
//                          AliasDefn Implementation
//>==---------------------------------------------------------------------------

AliasDefn* AliasDefn::create(AST::Context& ctx, SourceSpan span, 
                             const std::string& name, const Runes& runes, 
                             Type* type) {
    return new AliasDefn(span, name, runes, type);
}

//>==---------------------------------------------------------------------------
//                          StructDefn Implementation
//>==---------------------------------------------------------------------------

StructDefn* StructDefn::create(AST::Context& ctx, SourceSpan span, 
                               const std::string& name, const Runes& runes, 
                               Type* type) {
    return new StructDefn(span, name, runes, type);
}

StructDefn::~StructDefn() {
    for (FieldDefn* field : m_fields) {
        if (field)
            delete field;
    }

    m_fields.clear();
}

//>==---------------------------------------------------------------------------
//                          EnumDefn Implementation
//>==---------------------------------------------------------------------------

EnumDefn* EnumDefn::create(AST::Context& ctx, SourceSpan span, 
                           const std::string& name, const Runes& runes, 
                           Type* type) {
    return new EnumDefn(span, name, runes, type);
}

EnumDefn::~EnumDefn() {
    for (VariantDefn* variant : m_variants)
        delete variant;

    m_variants.clear();
}
