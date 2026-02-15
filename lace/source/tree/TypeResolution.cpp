//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/core/Diagnostics.hpp"
#include "lace/tree/Defn.hpp"
#include "lace/tree/Scope.hpp"
#include "lace/tree/TypeResolution.hpp"

using namespace lace;

TypeResolution::TypeResolution(Options& options) : m_options(options) {}

Result TypeResolution::resolveType(const QualType& type) const {
    switch (type->get_class()) {
        case Type::Array:
            return resolveType(static_cast<const ArrayType*>(type.get_type())->get_element_type());

        case Type::Deferred: {
            NamedDefn* named_defn = m_scope->get(static_cast<const DeferredType*>(
                type.get_type())->get_name());
            if (!named_defn)
                return false;

            TypeDefn* type_defn = dynamic_cast<TypeDefn*>(named_defn);
            if (!type_defn)
                return false;

            type.set_type(type_defn->get_type());
            return true;
        }

        case Type::Enum:
            return resolveType(
                static_cast<const EnumType*>(type.get_type())->get_underlying());

        case Type::Function: {
            const FunctionType* func_type = static_cast<const FunctionType*>(
                type.get_type());

            if (!resolveType(func_type->get_return_type()))
                return false;

            for (auto& param : func_type->get_params())
                if (!resolveType(param))
                    return false;

            return true;
        }

        case Type::Pointer:
            return resolveType(
                static_cast<const PointerType*>(type.get_type())->get_pointee());

        default:
            return true;
    }
}

void TypeResolution::visit(AST& ast) {
    m_ast = &ast;
    m_context = &ast.get_context();
    m_scope = ast.get_scope();

    for (Defn* defn : ast.get_defns())
        defn->accept(*this);
}

void TypeResolution::visit(VariableDefn& node) {
    if (!resolveType(node.get_type()))
        log::error("unresolved type: " + node.get_type().to_string(), 
            log::Span(m_ast->get_file(), node.get_span()));
}

void TypeResolution::visit(FunctionDefn& node) {
    if (!resolveType(node.get_type()))
        log::fatal("unresolved type: " + node.get_type().to_string(), 
            log::Span(m_ast->get_file(), node.get_span().start));

    const FunctionType* type = static_cast<const FunctionType*>(
        node.get_type().get_type());

    for (uint32_t i = 0, e = node.num_params(); i < e; ++i)
        node.get_params()[i]->set_type(type->get_param(i));
}

void TypeResolution::visit(FieldDefn& node) {
    if (!resolveType(node.get_type()))
        log::fatal("unresolved type: " + node.get_type().to_string(), 
            log::Span(m_ast->get_file(), node.get_span()));
}

void TypeResolution::visit(VariantDefn& node) {
    if (!resolveType(node.get_type()))
        log::fatal("unresolved type: " + node.get_type().to_string(), 
            log::Span(m_ast->get_file(), node.get_span()));
}

void TypeResolution::visit(StructDefn& node) {
    for (FieldDefn* field : node.get_fields())
        field->accept(*this);
}

void TypeResolution::visit(EnumDefn& node) {
    for (VariantDefn* variant : node.get_variants())
        variant->accept(*this);
}
