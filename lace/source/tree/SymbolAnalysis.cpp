//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/core/Diagnostics.h"
#include "lace/tree/AST.h"
#include "lace/tree/Defn.h"
#include "lace/tree/Expr.h"
#include "lace/tree/Stmt.h"
#include "lace/tree/SymbolAnalysis.h"
#include "lace/tree/Type.h"
#include "lace/tree/VisitorBase.h"

using namespace lace;

SymbolAnalysis::SymbolAnalysis(Options& options) : VisitorBase(options) {}

void SymbolAnalysis::visit(VariableDefn& node) {
    const log::Span span = { m_ast->get_file(), node.span() };
    Type* type = resolve_type(node.type());
    if (!type)
        log::fatal("unresolved type: " + node.type()->string(), span);
    
    node.set_type(type);

    VisitorBase::visit(node);
}

void SymbolAnalysis::visit(AccessExpr& node) {
    const log::Span span = log::Span(m_ast->get_file(), node.get_span());
    const std::string& name = node.name();

    VisitorBase::visit(node);

    // Check that the base type is a struct or a pointer to one.
    Type* base_type = node.base()->type();
    if (auto ptr = dynamic_cast<PointerType*>(base_type))
        base_type = ptr->pointee();

    StructType* struct_type = dynamic_cast<StructType*>(base_type);
    if (!struct_type)
        log::fatal("'.' base must be a struct or a pointer to one", span);

    // Resolve the struct definition from the base type.
    StructDefn* struct_defn = struct_type->defn();
    assert(struct_defn);

    // Resolve the target field from the struct definition.
    FieldDefn* field = struct_defn->get_field(name);
    if (!field)
        log::fatal("field '" + name + "' does not exist", span);

    node.set_field(field);
    node.set_type(field->type());
}

void SymbolAnalysis::visit(CallExpr& node) {
    VisitorBase::visit(node);

    // @Todo: maybe propogate function return type here.
}

void SymbolAnalysis::visit(CastExpr& node) {
    VisitorBase::visit(node);

    const log::Span span = { m_ast->get_file(), node.get_span() };
    Type* type = resolve_type(node.type());
    if (!type)
        log::fatal("unresolved type: " + node.type()->string(), span);
    
    node.set_type(type);
}

void SymbolAnalysis::visit(RefExpr& node) {
    const log::Span span = log::Span(m_ast->get_file(), node.get_span());
    const std::string& name = node.name();

    Scope* prev_scope = m_scope;

    for (Specifier& spec : node.specs()) {
        SpaceDefn* nspace = m_scope->get_namespace(spec.name);
        if (!nspace)
            log::fatal("unknown namespace: " + spec.name, span);

        spec.nspace = nspace;
        m_scope = nspace->scope();
    }

    NamedDefn* named_defn = m_scope->get(name);
    if (!named_defn)
        log::fatal("unresolved reference: " + name, span);

    ValueDefn* value_defn = dynamic_cast<ValueDefn*>(named_defn);
    if (!value_defn)
        log::fatal("invalid reference: " + name, span);

    node.set_defn(value_defn);
    node.set_type(value_defn->type());

    m_scope = prev_scope;
}

void SymbolAnalysis::visit(SizeofExpr& node) {
    const log::Span span = { m_ast->get_file(), node.get_span() };
    Type* type = resolve_type(node.target());
    if (!type)
        log::fatal("unresolved type: " + node.target()->string(), span);
    
    node.set_target(type);
}

void SymbolAnalysis::visit(StructInitExpr& node) {
    const log::Span span = { m_ast->get_file(), node.get_span() };
    Type* type = resolve_type(node.type());
    if (!type)
        log::fatal("unresolved type: " + node.type()->string(), span);
    
    node.set_type(type);

    VisitorBase::visit(node);

    // Check that the base type is a struct.
    StructType* struct_type = dynamic_cast<StructType*>(type);
    if (!struct_type)
        log::fatal("'.' base must be a struct or a pointer to one", span);

    // Resolve the struct definition from the base type.
    StructDefn* struct_defn = struct_type->defn();
    assert(struct_defn);

    // Ensure that each field referenced by the initializer exists in the struct.
    for (auto& [field, expr] : node.fields()) {
        if (!struct_defn->has_field(field))
            log::fatal("unknown field: " + field, span);
    }
}
