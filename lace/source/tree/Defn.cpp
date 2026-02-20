//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lace/tree/Defn.h"
#include "lace/tree/Expr.h"

using namespace lace;

LoadDefn* LoadDefn::create(
        AST::Context& ctx, SourceSpan span, const std::string& path) {
    return new LoadDefn(span, path);
}

NamedDefn::~NamedDefn() {
    for (Rune* rune : m_runes)
        delete rune;

    m_runes.clear();
}

VariableDefn::~VariableDefn() {
    if (has_init()) {
        delete m_init;
        m_init = nullptr;
    }
}

VariableDefn* VariableDefn::create(AST::Context& ctx, SourceSpan span, 
                                   const std::string& name, const std::vector<Rune*>& runes, 
                                   const QualType& type, Expr* init, 
                                   bool global) {
    return new VariableDefn(span, name, runes, type, init, global);
}

ParameterDefn* ParameterDefn::create(AST::Context& ctx, SourceSpan span, 
                                     const std::string& name, 
                                     const std::vector<Rune*>& runes, const QualType& type) {
    return new ParameterDefn(span, name, runes, type);
}

FunctionDefn::~FunctionDefn() {
    delete m_scope;
    m_scope = nullptr;

    for (ParameterDefn* param : m_params)
        delete param;

    m_params.clear();

    if (has_body()) {
        delete m_body;
        m_body = nullptr;
    }
}

FunctionDefn* FunctionDefn::create(AST::Context& ctx, SourceSpan span, 
                                   const std::string& name, const std::vector<Rune*> &runes, 
                                   const QualType& type, Scope* scope, 
                                   const Params& params, BlockStmt* body) {
    return new FunctionDefn(span, name, runes, type, scope, params, body);
}

FieldDefn* FieldDefn::create(AST::Context& ctx, SourceSpan span, 
                             const std::string& name, const std::vector<Rune*>& runes, 
                             const QualType& type, uint32_t index) {
    return new FieldDefn(span, name, runes, type, index);
}

VariantDefn* VariantDefn::create(AST::Context& ctx, SourceSpan span, 
                                 const std::string& name, const std::vector<Rune*>& runes, 
                                 const QualType& type, int64_t value) {
    return new VariantDefn(span, name, runes, type, value);
}

AliasDefn* AliasDefn::create(AST::Context& ctx, SourceSpan span, 
                             const std::string& name, const std::vector<Rune*>& runes, 
                             const Type* type) {
    return new AliasDefn(span, name, runes, type);
}

StructDefn::~StructDefn() {
    for (FieldDefn* field : m_fields)
        delete field;

    m_fields.clear();
}

StructDefn* StructDefn::create(AST::Context& ctx, SourceSpan span, 
                               const std::string& name, const std::vector<Rune*>& runes, 
                               const Type* type) {
    return new StructDefn(span, name, runes, type);
}

EnumDefn::~EnumDefn() {
    for (VariantDefn* variant : m_variants)
        delete variant;

    m_variants.clear();
}

EnumDefn* EnumDefn::create(AST::Context& ctx, SourceSpan span, 
                           const std::string& name, const std::vector<Rune*>& runes, 
                           const Type* type) {
    return new EnumDefn(span, name, runes, type);
}
