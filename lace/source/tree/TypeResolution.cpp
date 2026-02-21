//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/core/Diagnostics.h"
#include "lace/tree/AST.h"
#include "lace/tree/Defn.h"
#include "lace/tree/TypeResolution.h"

using namespace lace;

TypeResolution::TypeResolution(Options& options) : VisitorBase(options) {}

void TypeResolution::visit(VariableDefn& node) {
    const log::Span span = { m_ast->get_file(), node.get_span() };
    Type* type = resolve_type(node.type());
    if (!type)
        log::error("unresolved type: " + node.type()->string(), span);

    node.set_type(type);
}

void TypeResolution::visit(FunctionDefn& node) {
    const log::Span span = { m_ast->get_file(), node.get_span() };
    Type* type = resolve_type(node.type());
    if (!type)
        log::error("unresolved type: " + node.type()->string(), span);
    
    FunctionType* sig = dynamic_cast<FunctionType*>(type);
    assert(sig);

    node.set_type(sig);

    // The function's type has been resolved at this point, but the types of 
    // the parameter definitions may be outdated.

    // For each function parameter, propogate its type to the same one as in the function type.
    for (uint32_t i = 0, e = node.num_params(); i < e; ++i)
        node.params()[i]->set_type(sig->get_param(i));
}

void TypeResolution::visit(FieldDefn& node) {
    const log::Span span = { m_ast->get_file(), node.get_span() };
    Type* type = resolve_type(node.type());
    if (!type)
        log::error("unresolved type: " + node.type()->string(), span);
    
    node.set_type(type);
}

void TypeResolution::visit(VariantDefn& node) {
    const log::Span span = { m_ast->get_file(), node.get_span() };
    Type* type = resolve_type(node.type());
    if (!type)
        log::error("unresolved type: " + node.type()->string(), span);
    
    node.set_type(type);
}
