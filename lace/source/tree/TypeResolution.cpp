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
    if (!resolveType(node.get_type())) {
        log::error("unresolved type: " + node.get_type().to_string(), 
            log::Span(m_ast->get_file(), node.get_span()));
    }
}

void TypeResolution::visit(FunctionDefn& node) {
    if (!resolveType(node.get_type())) {
        log::fatal("unresolved type: " + node.get_type().to_string(), 
            log::Span(m_ast->get_file(), node.get_span().start));
    }

    // The function's type has been resolved at this point, but the types of the parameters may be
    // outdated.

    auto type = dynamic_cast<const FunctionType*>(node.get_type().get_type());
    assert(type);

    // For each function parameter, propogate its type to the same one as in the function type.
    for (uint32_t i = 0, e = node.num_params(); i < e; ++i)
        node.get_params()[i]->set_type(type->get_param(i));
}

void TypeResolution::visit(FieldDefn& node) {
    if (!resolveType(node.get_type())) {
        log::fatal("unresolved type: " + node.get_type().to_string(), 
            log::Span(m_ast->get_file(), node.get_span()));
    }
}

void TypeResolution::visit(VariantDefn& node) {
    if (!resolveType(node.get_type())) {
        log::fatal("unresolved type: " + node.get_type().to_string(), 
            log::Span(m_ast->get_file(), node.get_span()));
    }
}
