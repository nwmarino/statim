//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/tree/Defn.h"
#include "lace/tree/Expr.h"
#include "lace/tree/LIRCodegen.h"
#include "lace/tree/Type.h"

#include "lace/tree/VisitorBase.h"
#include "lir/graph/Function.h"
#include "lir/graph/Local.h"
#include "lir/graph/Type.h"

using namespace lace;

lir::Value* LIRCodegen::codegen_addressed_expression(const Expr* expr) {
    if (auto unary = dynamic_cast<const UnaryOp*>(expr)) {
        assert(unary->op() == UnaryOp::Dereference &&
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

lir::Value* LIRCodegen::codegen_addressed_access(const AccessExpr* expr) {
    lir::Value* ptr = nullptr;

    if (dynamic_cast<const PointerType*>(expr->base()->type())) {
        // If this access is functionally similar to C-style '->' access, then
        // we need to load the base to get at the underlying structure.
        ptr = codegen_valued_expression(expr->base());
    } else if (dynamic_cast<const StructType*>(expr->base()->type())) {
        ptr = codegen_addressed_expression(expr->base());
    } else {
        assert(false && "invalid type operand to field access!");
    }

    assert(ptr);

    lir::Type* type = lir::PointerType::get(m_cfg, to_lir_type(expr->type()));
    
    assert(expr->is_resolved());

    if (auto field = dynamic_cast<const FieldDefn*>(expr->field())) {
        return m_builder.build_access(type, ptr, lir::Integer::get(
            m_cfg, 
            lir::Type::get_i64(m_cfg), 
            field->get_index()
        ));
    } else if (auto method = dynamic_cast<const FunctionDefn*>(expr->field())) {
        lir::Function* func = m_funcs.at(method);
        assert(func);

        return func;
    }

    assert(false && "invalid access field type!");
}

lir::Value* LIRCodegen::codegen_addressed_reference(const RefExpr* expr) {
    assert(expr->is_resolved());

    if (auto func_defn = dynamic_cast<const FunctionDefn*>(expr->defn())) {
        assert(m_funcs.contains(func_defn));
        return m_funcs.at(func_defn);
    } else if (auto param_defn = dynamic_cast<const ParameterDefn*>(expr->defn())) {
        assert(m_func && "parameter reference outside a function!");

        lir::Local* local = m_func->get_local(expr->name());
        assert(local && "parameter does not exist!");
        
        return local;
    } else if (auto var_defn = dynamic_cast<const VariableDefn*>(expr->defn())) {
        if (var_defn->is_global()) {
            assert(m_globals.contains(var_defn));
            return m_globals.at(var_defn);
        } else {
            assert(m_func && "local reference not within a function!");

            lir::Local* local = m_func->get_local(expr->name());
            assert(local && "local variable does not exist!");

            return local;
        }
    }

    return nullptr;
}

lir::Value* LIRCodegen::codegen_addressed_subscript(const SubscriptExpr* expr) {
    lir::Value* ptr = nullptr;
    if (dynamic_cast<const PointerType*>(expr->base()->type())) {
        ptr = codegen_valued_expression(expr->base());
    } else {
        assert(false && "invalid type operand to subscript!");
    }

    lir::Value* index = codegen_valued_expression(expr->index());
    assert(ptr);
    assert(index);

    return m_builder.build_offptr(
        lir::PointerType::get(m_cfg, to_lir_type(expr->type())), 
        ptr,
        index
    );
}

lir::Value* LIRCodegen::codegen_addressed_dereference(const UnaryOp* expr) {
    lir::Value* rvalue = codegen_valued_expression(expr->expr());
    assert(rvalue);

    return rvalue;
}

lir::Value* LIRCodegen::codegen_struct_init(const StructInitExpr* expr) {
    lir::Value* dest = nullptr;
    lir::Type* type = to_lir_type(expr->type());

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

    auto struct_type = dynamic_cast<const StructType*>(expr->type());
    assert(struct_type);

    const StructDefn* struct_defn = struct_type->defn();

    for (auto& [name, init] : expr->fields()) {
        const FieldDefn* field = struct_defn->get_field(name);
        assert(field);

        lir::Type* field_type = to_lir_type(field->type());

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
