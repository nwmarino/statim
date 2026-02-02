//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/codegen/LIRCodegen.hpp"
#include "lace/tree/AST.hpp"
#include "lace/tree/Defn.hpp"
#include "lace/tree/Type.hpp"

#include "lir/graph/Type.hpp"

using namespace lace;

void LIRCodegen::run() {
    // Definitions which were imported by a load should only be partially
    // defined i.e. lowered, except for types, which should always be fully
    // defined.

    std::vector<Defn*> partials = {};
    std::vector<Defn*> defns = {};
    std::vector<TypeDefn*> types = {};

    for (Defn *D : m_ast->get_loaded()) {
        auto *TD = dynamic_cast<TypeDefn*>(D);
        if (TD) {
            types.push_back(TD);
        } else {
            partials.push_back(D);
        }
    }

    for (Defn *D : m_ast->get_defns()) {
        auto  *TD = dynamic_cast<TypeDefn*>(D);
        if (TD) {
            types.push_back(TD);
        } else {
            defns.push_back(D);
        }
    }

    // Lower all type definitions, but don't fill them out incase their fields
    // use a type not defined yet.
    for (TypeDefn *TD : types)
        codegen_initial_definition(TD);
    
    // Fill out all type definitions, now that all type information is 
    // available.
    for (TypeDefn *TD : types)
        codegen_lowered_definition(TD);

    // Lower all imported definitions. This is the last time we touch them.
    for (Defn *D : partials)
        codegen_initial_definition(D);

    for (Defn *D : defns)
        codegen_initial_definition(D);

    for (Defn *D : defns)
        codegen_lowered_definition(D);
}

lir::Type *LIRCodegen::to_lir_type(const QualType &type) {
    switch (type->get_class()) {
        case Type::Alias:
            return to_lir_type(static_cast<const AliasType*>
                (type.get_type())->get_underlying());
        
        case Type::Array: {
            auto array = static_cast<const ArrayType*>(type.get_type());
            return lir::ArrayType::get(m_cfg, to_lir_type(
                array->get_element_type()), array->get_size());
        }

        case Type::Builtin: {
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

        case Type::Deferred:
            assert(false && "cannot lower deferred type!");

        case Type::Enum:
            return to_lir_type(static_cast<const EnumType*>(
                type.get_type())->get_underlying());

        case Type::Function: {
            auto sig = static_cast<const FunctionType*>(type.get_type());
            std::vector<lir::Type*> args(sig->num_params(), nullptr);
            for (uint32_t i = 0; i < sig->num_params(); ++i)
                args[i] = to_lir_type(sig->get_param(i));

            return lir::FunctionType::get(
                m_cfg, args, to_lir_type(sig->get_return_type()));
        }

        case Type::Pointer:
            return lir::PointerType::get(m_cfg, to_lir_type(
                static_cast<const PointerType*>(type.get_type())->get_pointee()));

        case Type::Struct:
            return lir::StructType::get(m_cfg, 
                static_cast<const StructType*>(type.get_type())->to_string());
    }
}

lir::Function *LIRCodegen::get_function(
        const std::string &name, lir::Type *result, 
        const lir::FunctionType::Params &params) {
    lir::Function *func = m_cfg.get_function(name);
    if (func)
        return func;

    std::vector<lir::Parameter*> parameters(params.size(), nullptr);

    for (uint32_t i = 0; i < params.size(); ++i)
        parameters[i] = lir::Parameter::create(params[i]);

    return lir::Function::create(
        m_cfg, 
        lir::Function::LinkageType::Public, 
        lir::FunctionType::get(m_cfg, params, result),
        name, 
        parameters
    );
}

lir::Value *LIRCodegen::inject_comparison(lir::Value *value) {
    lir::Type *type = value->get_type();
    
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
