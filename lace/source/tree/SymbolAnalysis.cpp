//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/core/Diagnostics.hpp"
#include "lace/tree/AST.hpp"
#include "lace/tree/Defn.hpp"
#include "lace/tree/Expr.hpp"
#include "lace/tree/Stmt.hpp"
#include "lace/tree/SymbolAnalysis.hpp"
#include "lace/tree/Type.hpp"
#include "lace/tree/VisitorBase.hpp"

using namespace lace;

SymbolAnalysis::SymbolAnalysis(Options& options) : VisitorBase(options) {}

void SymbolAnalysis::visit(VariableDefn& node) {
    if (!resolveType(node.get_type())) {
        log::error("unresolved type: " + node.get_type().to_string(), 
            log::Span(m_ast->get_file(), node.get_span()));
    }

    VisitorBase::visit(node);
}

void SymbolAnalysis::visit(AccessExpr& node) {
    const log::Span span = log::Span(m_ast->get_file(), node.get_span());
    const std::string& name = node.get_name();

    VisitorBase::visit(node);

    // Check that the base type is a struct.
    QualType base_type = node.get_base()->get_type();
    if (base_type->isPointer())
        base_type = static_cast<const PointerType*>(base_type.get_type())->get_pointee();

    if (!base_type->isStruct())
        log::fatal("'.' base must be a struct or a pointer to one", span);

    // Resolve the struct definition from the base type.
    const StructDefn* struct_defn = static_cast<const StructType*>(base_type.get_type())->get_defn();

    // Resolve the target field from the struct definition.
    const FieldDefn* field = struct_defn->get_field(name);
    if (!field)
        log::fatal("field '" + name + "' does not exist", span);

    node.set_field(field);
    node.set_type(field->get_type());
}

void SymbolAnalysis::visit(CallExpr& node) {
    VisitorBase::visit(node);

    // @Todo: maybe propogate function return type here.
}

void SymbolAnalysis::visit(CastExpr& node) {
    VisitorBase::visit(node);

    if (!resolveType(node.get_type()))
        log::fatal("unresolved type: " + node.get_type().to_string(), 
            log::Span(m_ast->get_file(), node.get_span()));
}

void SymbolAnalysis::visit(RefExpr& node) {
    const log::Span span = log::Span(m_ast->get_file(), node.get_span());
    const std::string& name = node.get_name();

    NamedDefn* named_defn = m_scope->get(name);
    if (!named_defn)
        log::fatal("unresolved reference: " + name, span);

    ValueDefn* value_defn = dynamic_cast<ValueDefn*>(named_defn);
    if (!value_defn)
        log::fatal("invalid reference: " + name, span);

    node.set_defn(value_defn);
    node.set_type(value_defn->get_type());
}

void SymbolAnalysis::visit(SizeofExpr& node) {
    if (!resolveType(node.get_target_type())) {
        log::fatal("unresolved type: " + node.get_target_type().to_string(), 
            log::Span(m_ast->get_file(), node.get_span()));
    }
}
