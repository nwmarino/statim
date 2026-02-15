//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/codegen/LIRCodegen.hpp"
#include "lace/core/Diagnostics.hpp"
#include "lace/tree/AST.hpp"
#include "lace/tree/Defn.hpp"
#include "lace/tree/Type.hpp"

#include "lir/graph/Function.hpp"
#include "lir/graph/Type.hpp"

using namespace lace;

lir::Value *LIRCodegen::codegen_addressed_expression(const Expr *expr) {
    switch (expr->get_kind()) {
        case Expr::Unary: {
            auto unary = static_cast<const UnaryOp*>(expr);
            assert(unary->get_operator() == UnaryOp::Dereference &&
                "cannot generate an address from non-dereference unary op!");
            
            return codegen_addressed_dereference(unary);
        }

        case Expr::Access:
            return codegen_addressed_access(static_cast<const AccessExpr*>(expr));

        case Expr::Ref:
            return codegen_addressed_reference(static_cast<const RefExpr*>(expr));

        case Expr::Subscript:
            return codegen_addressed_subscript(static_cast<const SubscriptExpr*>(expr));

        case Expr::Call:
            return codegen_function_call(static_cast<const CallExpr*>(expr));

        default:
            return nullptr;
    }
}

lir::Value *LIRCodegen::codegen_addressed_access(const AccessExpr *expr) {
    lir::Value *ptr = nullptr;

    const Expr *base = expr->get_base();
    if (base->get_type()->isClass(Type::Class::Pointer)) {
        // If this access is functionally similar to C-style '->' access, then
        // we need to load the base to get at the underlying structure.
        ptr = codegen_valued_expression(base);
    } else if (base->get_type()->isClass(Type::Class::Struct)) {
        ptr = codegen_addressed_expression(base);
    } else {
        log::fatal("bad type operand to '.': " + base->get_type().to_string(),
            log::Span(m_ast->get_file(), expr->get_span()));
    }

    assert(ptr);

    lir::Type *type = lir::PointerType::get(m_cfg, to_lir_type(expr->get_type()));

    return m_builder.build_access(type, ptr, lir::Integer::get(
        m_cfg, lir::Type::get_i64(m_cfg), expr->get_field()->get_index()
    ));
}

lir::Value *LIRCodegen::codegen_addressed_reference(const RefExpr *expr) {
    assert(expr->get_defn());

    switch (expr->get_defn()->get_kind()) {
        case Defn::Function: {
            lir::Function *func = m_cfg.get_function(expr->get_name());
            assert(func && "function does not exist!");

            return func;
        }

        case Defn::Parameter: {
            assert(m_func && "parameter reference outside a function!");

            lir::Local *local = m_func->get_local(expr->get_name());
            assert(local && "parameter does not exist!");
            
            return local;
        }

        case Defn::Variable: {
            auto var = static_cast<const VariableDefn*>(expr->get_defn());

            if (var->is_global()) {
                lir::Global *global = m_cfg.get_global(expr->get_name());
                assert(global && "global variable does not exist!");

                return global;
            } else {
                assert(m_func && "local reference not within a function!");

                lir::Local *local = m_func->get_local(expr->get_name());
                assert(local && "local variable does not exist!");

                return local;
            }
        }

        default:
            assert(false && "unable to generate address reference!");
    }
}

lir::Value *LIRCodegen::codegen_addressed_subscript(const SubscriptExpr *expr) {
    lir::Value *ptr = nullptr;
    const Expr *base = expr->get_base();

    if (base->get_type()->isClass(Type::Class::Array)) {
        ptr = codegen_addressed_expression(base);
    } else if (base->get_type()->isClass(Type::Class::Pointer)) {
        ptr = codegen_valued_expression(base);
    } else {
        log::fatal("invalid [] type operand: " + base->get_type().to_string(), 
            log::Span(m_ast->get_file(), expr->get_span()));
    }

    lir::Value *index = codegen_valued_expression(expr->get_index());
    assert(ptr);
    assert(index);

    lir::Type *type = lir::PointerType::get(
        m_cfg, to_lir_type(expr->get_type()));

    if (base->get_type()->isClass(Type::Class::Array)) {
        // If the base is an array, we want to access an element, not 
        // manipulate the address.
        return m_builder.build_access(type, ptr, index);
    } else {
        return m_builder.build_offptr(type, ptr, index);
    }
}

lir::Value *LIRCodegen::codegen_addressed_dereference(const UnaryOp *expr) {
    lir::Value *rvalue = codegen_valued_expression(expr->get_expr());
    assert(rvalue);

    return rvalue;
}
