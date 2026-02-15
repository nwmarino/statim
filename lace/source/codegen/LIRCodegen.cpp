//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/tree/AST.h"
#include "lace/tree/Defn.h"
#include "lace/tree/LIRCodegen.h"
#include "lace/tree/Type.h"

#include "lir/graph/Type.hpp"

using namespace lace;

void LIRCodegen::run() {
    // Definitions which were imported by a load should only be partially
    // defined i.e. lowered, except for types, which should always be fully
    // defined.

    std::vector<Defn*> partials = {};
    std::vector<Defn*> defns = {};
    std::vector<TypeDefn*> types = {};

    for (Defn* defn : m_ast->get_loaded()) {
        auto type_defn = dynamic_cast<TypeDefn*>(defn);
        if (type_defn) {
            types.push_back(type_defn);
        } else {
            partials.push_back(defn);
        }
    }

    for (Defn* defn : m_ast->get_defns()) {
        auto type_defn = dynamic_cast<TypeDefn*>(defn);
        if (type_defn) {
            types.push_back(type_defn);
        } else {
            defns.push_back(defn);
        }
    }

    // Lower all type definitions, but don't fill them out incase their fields
    // use a type not defined yet.
    for (TypeDefn* type_defn : types)
        codegen_initial_definition(type_defn);
    
    // Fill out all type definitions, now that all type information is 
    // available.
    for (TypeDefn* type_defn : types)
        codegen_lowered_definition(type_defn);

    // Lower all imported definitions. This is the last time we touch them.
    for (Defn* defn : partials)
        codegen_initial_definition(defn);

    for (Defn* defn : defns)
        codegen_initial_definition(defn);

    for (Defn* defn : defns)
        codegen_lowered_definition(defn);
}

lir::Type* LIRCodegen::to_lir_type(const QualType& type) {
    switch (type->getClass()) {
        case Type::Class::Alias:
            return to_lir_type(static_cast<const AliasType*>
                (type.get_type())->get_underlying());
        
        case Type::Class::Array: {
            auto array = static_cast<const ArrayType*>(type.get_type());
            return lir::ArrayType::get(m_cfg, to_lir_type(
                array->get_element_type()), array->get_size());
        }

        case Type::Class::Builtin: {
            auto builtin = static_cast<const BuiltinType*>(type.get_type());

            switch (builtin->get_kind()) {
                case BuiltinType::Void:
                    return lir::Type::get_void(m_cfg);
                case BuiltinType::Bool:
                case BuiltinType::Char:
                case BuiltinType::Int8:
                case BuiltinType::UInt8:
                    return lir::Type::get_i8(m_cfg);
                case BuiltinType::Int16:
                case BuiltinType::UInt16:
                    return lir::Type::get_i16(m_cfg);
                case BuiltinType::Int32:
                case BuiltinType::UInt32:
                    return lir::Type::get_i32(m_cfg);
                case BuiltinType::Int64:
                case BuiltinType::UInt64:
                    return lir::Type::get_i64(m_cfg);
                case BuiltinType::Float32:
                    return lir::Type::get_f32(m_cfg);
                case BuiltinType::Float64:
                    return lir::Type::get_f64(m_cfg);
            }

            __builtin_unreachable();
        }

        case Type::Class::Deferred:
            assert(false && "cannot lower deferred type!");

        case Type::Class::Enum:
            return to_lir_type(static_cast<const EnumType*>(
                type.get_type())->get_underlying());

        case Type::Class::Function: {
            auto func_type = static_cast<const FunctionType*>(type.get_type());
            std::vector<lir::Type*> args = {};
            args.reserve(func_type->num_params());

            lir::Type* return_type = to_lir_type(func_type->get_return_type());
            if (!m_mach.is_scalar(return_type)) {
                // If the return type is an aggregate, then it must be passed as the first argument
                // via hidden pointer. The return type then becomes void.
                args.push_back(lir::PointerType::get(m_cfg, return_type));
                return_type = lir::VoidType::get(m_cfg);
            }

            for (uint32_t i = 0; i < func_type->num_params(); ++i) {
                lir::Type* param_type = to_lir_type(func_type->get_param(i));
                if (m_mach.is_scalar(param_type)) {
                    args.push_back(param_type);
                } else {
                    // If the parameter type is an aggregate, then it is passed via hidden pointer.
                    args.push_back(lir::PointerType::get(m_cfg, param_type));
                }
            }

            return lir::FunctionType::get(m_cfg, args, return_type);
        }

        case Type::Class::Pointer:
            return lir::PointerType::get(m_cfg, to_lir_type(
                static_cast<const PointerType*>(type.get_type())->get_pointee()));

        case Type::Class::Struct:
            return lir::StructType::get(m_cfg, 
                static_cast<const StructType*>(type.get_type())->to_string());
    }
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
