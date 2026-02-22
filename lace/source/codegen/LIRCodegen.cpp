//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/tree/AST.h"
#include "lace/tree/Defn.h"
#include "lace/tree/LIRCodegen.h"
#include "lace/tree/Type.h"

#include "lir/graph/Type.h"

#include <set>

using namespace lace;

void LIRCodegen::run() {
    // Defined types are lowered first, as full type information needs to be
    // available before the lowering of other definitions.
    std::set<TypeDefn*> types = {};
    for (Defn* defn : m_ast->defns()) {
        auto type = dynamic_cast<TypeDefn*>(defn);
        if (!type)
            continue;

        // Types are only initially defined first. For example, if the field of 
        // a structure references another structure that hasn't been lowered
        // yet, then it will be unresolved.
        codegen_initial_definition(type);
        types.insert(type);
    }

    for (TypeDefn* type : types)
        codegen_lowered_definition(type);

    std::set<Defn*> defns = {};
    for (Defn* defn : m_ast->defns()) {
        // Skip type definitions since they've been lowered by now.
        if (dynamic_cast<TypeDefn*>(defn))
            continue;

        codegen_initial_definition(defn);

        // Only record non-imported definitions to be fully lowered.
        if (defn->origin() == m_ast)
            defns.insert(defn);
    }

    for (Defn* defn : defns)
        codegen_lowered_definition(defn);
}

lir::Type* LIRCodegen::to_lir_type(const Type* type) {
    if (auto alias = dynamic_cast<const AliasType*>(type)) {
        return to_lir_type(static_cast<const AliasType*>(type)->aliased());
    } else if (auto builtin = dynamic_cast<const BuiltinType*>(type)) {
        switch (builtin->kind()) 
        {
        case BuiltinType::Kind::Void:
            return lir::Type::get_void(m_cfg);
        case BuiltinType::Kind::Bool:
        case BuiltinType::Kind::Char:
        case BuiltinType::Kind::Int8:
        case BuiltinType::Kind::UInt8:
            return lir::Type::get_i8(m_cfg);
        case BuiltinType::Kind::Int16:
        case BuiltinType::Kind::UInt16:
            return lir::Type::get_i16(m_cfg);
        case BuiltinType::Kind::Int32:
        case BuiltinType::Kind::UInt32:
            return lir::Type::get_i32(m_cfg);
        case BuiltinType::Kind::Int64:
        case BuiltinType::Kind::UInt64:
            return lir::Type::get_i64(m_cfg);
        case BuiltinType::Kind::Float32:
            return lir::Type::get_f32(m_cfg);
        case BuiltinType::Kind::Float64:
            return lir::Type::get_f64(m_cfg);
        }

        assert(false);
    } else if (auto enumeration = dynamic_cast<const EnumType*>(type)) {
        return to_lir_type(enumeration->underlying());
    } else if (auto sig = dynamic_cast<const FunctionType*>(type)) {
        std::vector<lir::Type*> args = {};
        args.reserve(sig->num_params());

        lir::Type* return_type = to_lir_type(sig->result());
        if (!m_mach.is_scalar(return_type)) {
            // If the return type is an aggregate, then it must be passed as the first argument
            // via hidden pointer. The return type then becomes void.
            args.push_back(lir::PointerType::get(m_cfg, return_type));
            return_type = lir::VoidType::get(m_cfg);
        }

        for (uint32_t i = 0; i < sig->num_params(); ++i) {
            lir::Type* param_type = to_lir_type(sig->get_param(i));
            if (m_mach.is_scalar(param_type)) {
                args.push_back(param_type);
            } else {
                // If the parameter type is an aggregate, then it is passed via hidden pointer.
                args.push_back(lir::PointerType::get(m_cfg, param_type));
            }
        }

        return lir::FunctionType::get(m_cfg, args, return_type);
    } else if (auto ptr = dynamic_cast<const PointerType*>(type)) {
        return lir::PointerType::get(m_cfg, to_lir_type(ptr->pointee()));
    } else if (auto structure = dynamic_cast<const StructType*>(type)) {
        return lir::StructType::get(m_cfg, structure->string());
    }

    assert(false && "failed to lower type!");
}

void LIRCodegen::enter_namespace(const SpaceDefn* space) {
    m_namespaces.push_back(space);
}

void LIRCodegen::exit_namespace() {
    m_namespaces.pop_back();
}

std::string LIRCodegen::get_namespace_prefix() const {
    std::string res = "";
    for (const SpaceDefn* space : m_namespaces)
        res += space->name() + '.';

    return res;
}

lir::Function* LIRCodegen::get_function(const std::string& name, lir::Type* result, 
                                        const std::vector<lir::Type*>& args) {
    lir::Function* func = m_cfg.get_function(name);
    if (func)
        return func;

    std::vector<lir::Parameter*> params(args.size(), nullptr);
    for (uint32_t i = 0; i < args.size(); ++i)
        params[i] = lir::Parameter::create(args[i]);

    return lir::Function::create(
        m_cfg, 
        lir::Function::LinkageType::Public, 
        lir::FunctionType::get(m_cfg, args, result),
        name, 
        params
    );
}

lir::Function* LIRCodegen::getIntrinsicCopy() {
    return get_function("__copy", lir::VoidType::get(m_cfg), {
        lir::PointerType::get(m_cfg, lir::VoidType::get(m_cfg)),
        lir::PointerType::get(m_cfg, lir::VoidType::get(m_cfg)),
        lir::IntegerType::get(m_cfg, 64),
    });
}

lir::Value* LIRCodegen::inject_comparison(lir::Value* value) {
    lir::Type* type = value->get_type();
    
    if (type->is_integer_type(8)) {
        return value;
    } else if (type->is_integer_type()) {
        return m_builder.build_cmp_ine(value, lir::Integer::get_zero(m_cfg, type));
    } else if (type->is_float_type()) {
        return m_builder.build_cmp_fne(value, lir::Float::get_zero(m_cfg, type));
    } else if (type->is_pointer_type()) {
        return m_builder.build_cmp_ine(value, lir::Null::get(m_cfg, type));
    }

    assert(false && "value cannot be reduced to a boolean!");
}
