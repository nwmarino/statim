//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/core/Diagnostics.h"
#include "lace/tree/AST.h"
#include "lace/tree/Defn.h"
#include "lace/tree/Type.h"
#include "lace/tree/TypeResolution.h"

using namespace lace;

TypeResolution::TypeResolution(Options& options) : VisitorBase(options) {}

void TypeResolution::visit(VariableDefn& node) {
    const log::Span span = { m_ast->get_file(), node.span() };
    Type* type = resolve_type(node.type());
    if (!type)
        log::fatal("unresolved type: " + node.type()->string(), span);

    node.set_type(type);
}

void TypeResolution::visit(FunctionDefn& node) {
    const log::Span span = { m_ast->get_file(), node.span() };

    m_scope = node.scope();

    Type* type = resolve_type(node.type());
    if (!type)
        log::fatal("unresolved type: " + node.type()->string(), span);
    
    FunctionType* sig = dynamic_cast<FunctionType*>(type);
    assert(sig);

    node.set_type(sig);

    // The function's type has been resolved at this point, but the types of 
    // the parameter definitions may be outdated.

    uint32_t i = 0;

    if (node.has_receiver()) {
        assert(sig->num_params() >= 1);
        node.receiver()->set_type(sig->get_param(0));

        // This function is a method to some structure. We must try and resolve
        // that structure and add this is as a method.
        Type* receiver_type = node.get_receiver_type();
        assert(receiver_type);

        StructType* struct_type = dynamic_cast<StructType*>(receiver_type);
        if (!struct_type)
            log::fatal("cannot define a receiver for non-struct", span);

        StructDefn* struct_defn = struct_type->defn();
        assert(struct_defn);

        // Check that a method with the same name doesn't already exist.
        if (struct_defn->has_method(node.name()))
            log::fatal("method " + node.name() + " already exists", span);
        
        struct_defn->methods().push_back(&node);
        i = 1;
    }

    for (const uint32_t e = node.num_params(); i < e; ++i) {
        ParameterDefn* param = node.get_param(i);
        param->set_type(sig->get_param(i));
    }

    m_scope = m_scope->parent();
}

void TypeResolution::visit(FieldDefn& node) {
    const log::Span span = { m_ast->get_file(), node.span() };
    Type* type = resolve_type(node.type());
    if (!type)
        log::fatal("unresolved type: " + node.type()->string(), span);
    
    node.set_type(type);
}

void TypeResolution::visit(VariantDefn& node) {
    const log::Span span = { m_ast->get_file(), node.span() };
    Type* type = resolve_type(node.type());
    if (!type)
        log::fatal("unresolved type: " + node.type()->string(), span);
    
    node.set_type(type);
}
