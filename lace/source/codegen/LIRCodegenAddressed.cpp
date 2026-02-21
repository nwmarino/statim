//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/core/Diagnostics.h"
#include "lace/tree/AST.h"
#include "lace/tree/Defn.h"
#include "lace/tree/Expr.h"
#include "lace/tree/LIRCodegen.h"
#include "lace/tree/Type.h"

#include "lir/graph/Function.h"
#include "lir/graph/Local.h"
#include "lir/graph/Type.h"

using namespace lace;

lir::Value *LIRCodegen::codegen_addressed_expression(const Expr *expr) {
    if (auto unary = dynamic_cast<const UnaryOp*>(expr)) {
        assert(unary->get_operator() == UnaryOp::Dereference &&
            "cannot generate an address from non-dereference unary op!");
        
        return codegen_addressed_dereference(unary);
    } else if (auto access = dynamic_cast<const AccessExpr*>(expr)) {
        return codegen_addressed_access(access);
    } else if (auto ref = dynamic_cast<const RefExpr*>(expr)) {
        return codegen_addressed_reference(ref);
    } else if (auto subscript = dynamic_cast<const SubscriptExpr*>(expr)) {
        return codegen_addressed_subscript(subscript);
    } else if (auto call = dynamic_cast<const CallExpr*>(expr)) {
        return codegen_function_call(call);
    } else if (auto init = dynamic_cast<const StructInitExpr*>(expr)) {
        return codegen_struct_init(init);
    }

    return nullptr;
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
        log::fatal("bad type operand to '.': " + base->get_type().string(),
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
        log::fatal("invalid [] type operand: " + base->get_type().string(), 
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

lir::Value* LIRCodegen::codegen_addressed_dereference(const UnaryOp* expr) {
    lir::Value* rvalue = codegen_valued_expression(expr->get_expr());
    assert(rvalue);

    return rvalue;
}

lir::Value* LIRCodegen::codegen_struct_init(const StructInitExpr* expr) {
    lir::Value* dest = nullptr;
    lir::Type* type = to_lir_type(expr->get_type());

    if (m_state.place) {
        dest = m_state.place;
    } else {
        dest = lir::Local::create(
            m_cfg, 
            type, 
            std::to_string(m_cfg.get_def_id()),
            m_func
        );
    }

    const StructDefn* defn = static_cast<const StructType*>(
        expr->get_type().getType())->getDefn();

    for (auto& [name, init] : expr->fields()) {
        const FieldDefn* field = defn->get_field(name);
        assert(field);

        lir::Type* field_type = to_lir_type(field->get_type());

        lir::Value* ptr = m_builder.build_access(
            lir::PointerType::get(m_cfg, field_type), 
            dest, 
            lir::Integer::get(m_cfg, lir::IntegerType::get(m_cfg, 64), field->get_index())
        );

        if (m_mach.is_scalar(field_type)) {
            lir::Value* value = codegen_valued_expression(init);
            assert(value);

            m_builder.build_store(value, ptr);
        } else {
            lir::Value* prev_place = m_state.place;
            
            m_state.place = ptr;

            lir::Value* value = codegen_addressed_expression(init);
            if (value != ptr) {
                lir::Function* copy = getIntrinsicCopy();

                m_builder.build_call(copy, {
                    ptr,
                    value,
                    lir::Integer::get(m_cfg, lir::IntegerType::get(m_cfg, 64), m_mach.get_type_size(field_type) / 8)
                });
            }

            m_state.place = prev_place;
        }
    }

    return dest;
}
