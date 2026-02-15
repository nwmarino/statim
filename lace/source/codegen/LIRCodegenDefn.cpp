//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/codegen/LIRCodegen.hpp"
#include "lace/core/Diagnostics.hpp"
#include "lace/tree/Defn.hpp"
#include "lace/tree/Type.hpp"

#include "lir/graph/Constant.hpp"
#include "lir/graph/Function.hpp"
#include "lir/graph/Parameter.hpp"
#include "lir/graph/Type.hpp"

using namespace lace;

void LIRCodegen::codegen_initial_definition(const Defn* defn) {
    switch (defn->get_kind()) {
        case Defn::Function:
            codegenInitialFunction(static_cast<const FunctionDefn*>(defn));
            break;
        case Defn::Struct:
            codegen_initial_structure(static_cast<const StructDefn*>(defn));
            break;
        case Defn::Variable:
            codegen_initial_global(static_cast<const VariableDefn*>(defn));
            break;
        default:
            break;
    }
}

void LIRCodegen::codegen_lowered_definition(const Defn* defn) {
    switch (defn->get_kind()) {
        case Defn::Function:
            codegen_lowered_function(static_cast<const FunctionDefn*>(defn));
            break;
        case Defn::Struct:
            codegen_lowered_structure(static_cast<const StructDefn*>(defn));
            break;
        case Defn::Variable:
            codegen_lowered_global(static_cast<const VariableDefn*>(defn));
            break;
        default:
            break;
    }
}

lir::Function* LIRCodegen::codegenInitialFunction(const FunctionDefn* defn) {
    auto linkage = lir::Function::LinkageType::Private;
    if (defn->has_rune(Rune::Public))
        linkage = lir::Function::LinkageType::Public;

    std::vector<lir::Parameter*> params = {};
    params.reserve(defn->num_params());

    lir::Type* return_type = to_lir_type(defn->get_return_type());
    if (!m_mach.is_scalar(return_type)) {
        // Return type is an aggregate, so it is passed via hidden pointer as the first parameter.
        params.push_back(lir::Parameter::create(
            lir::PointerType::get(m_cfg, return_type), 
            "ret.ptr", 
            lir::Parameter::Trait::ARet
        ));
    }

    for (const ParameterDefn* param : defn->get_params()) {
        auto trait = lir::Parameter::Trait::None;

        std::string name = param->get_name();
        if (name == "_") {
            // Unnamed parameters i.e. '_' should be cleared at this point. 
            name.clear();
        }

        lir::Type* type = to_lir_type(param->get_type());
        if (!m_mach.is_scalar(type)) {
            // Parameter type is an aggregate, so it should be passed "by value" via a hidden ptr.
            type = lir::PointerType::get(m_cfg, type);
            trait = lir::Parameter::Trait::Byval;
        }

        params.push_back(lir::Parameter::create(type, name, trait));
    }

    lir::FunctionType* func_type = dynamic_cast<lir::FunctionType*>(to_lir_type(defn->get_type()));
    assert(func_type);

    return lir::Function::create(
        m_cfg, 
        linkage, 
        func_type,
        defn->get_name(), 
        params
    );
}

lir::Function* LIRCodegen::codegen_lowered_function(const FunctionDefn* defn) {
    lir::Function* func = m_cfg.get_function(defn->get_name());
    assert(func && "function does not exist!");

    // Skip functions without bodies.
    if (!defn->has_body())
        return func;

    m_func = func;

    lir::BasicBlock* entry = lir::BasicBlock::create(m_func);
    m_builder.set_insert(entry);

    for (uint32_t i = 0, e = func->num_params(); i < e; ++i) {
        lir::Parameter* param = func->get_param(i);
        lir::Type* type = param->get_type();

        if (param->hasTrait(lir::Parameter::Trait::ARet)) {
            continue;
        } else if (param->hasTrait(lir::Parameter::Trait::Byval)) {
            type = dynamic_cast<lir::PointerType*>(type)->get_pointee();
            assert(type);

            lir::Local* local = lir::Local::create(
                m_cfg, 
                type, 
                param->get_name(),
                func
            );

            lir::Function* copy = get_function("__copy", lir::VoidType::get(m_cfg), {
                lir::PointerType::get(m_cfg, lir::Type::get_void(m_cfg)),
                lir::PointerType::get(m_cfg, lir::Type::get_void(m_cfg)),
                lir::Type::get_i64(m_cfg),
            });

            m_builder.build_call(copy, { 
                local, 
                param, 
                lir::Integer::get(m_cfg, lir::Type::get_i64(m_cfg), m_mach.get_type_size(type) / 8),
            });
        } else {
            lir::Local* local = lir::Local::create(
                m_cfg, 
                type, 
                param->get_name(), 
                func
            );

            m_builder.build_store(param, local);
        }
    }

    codegen_statement(defn->get_body());

    if (!m_builder.get_insert()->terminates()) {
        if (!m_func->get_type()->has_result()) {
            m_builder.build_ret();
        } else {
            log::warn("non-void function does not always return a value", 
                log::Span(m_cfg.get_filename(), defn->get_span()));
        }
    }

    m_func = nullptr;
    m_builder.clear_insert();
    return func;
}

lir::Global* LIRCodegen::codegen_initial_global(const VariableDefn* defn) {
    auto linkage = lir::Global::LinkageType::Private;
    if (defn->has_rune(Rune::Public))
        linkage = lir::Global::LinkageType::Public;

    return lir::Global::create(
        m_cfg, 
        to_lir_type(defn->get_type()), 
        linkage, 
        defn->get_name(),
        // @Todo: for now, all lowered globals will be mutable. for the case
        // of arrays like [5]mut s64, where the elements are mutable, but the
        // array itself is not, we need some special semantics here.
        //
        // Cause if we had it as immutable, then the data would be put in 
        // read-only, and thus it wouldn't let us mutate the elements like we
        // should be able to.
        false /* !node.get_type().is_mut(), */
    );
}

lir::Global* LIRCodegen::codegen_lowered_global(const VariableDefn* defn) {
    lir::Global* global = m_cfg.get_global(defn->get_name());
    assert(global);

    if (!defn->has_init())
        return global;

    lir::Value* value = codegen_valued_expression(defn->get_init());
    assert(value);
    
    lir::Constant* init = dynamic_cast<lir::Constant*>(value);
    assert(init && "global is not initialized with a constant!");

    global->set_initializer(init);
    return global;
}

lir::StructType* LIRCodegen::codegen_initial_structure(const StructDefn* defn) {
    return lir::StructType::create(m_cfg, defn->get_name(), {});
}

lir::StructType* LIRCodegen::codegen_lowered_structure(const StructDefn* defn) {
    lir::StructType* type = lir::StructType::get(m_cfg, defn->get_name());
    assert(type && "type does not exist!");

    for (const FieldDefn* field : defn->get_fields())
        type->append_field(to_lir_type(field->get_type()));

    return type;
}

lir::Local* LIRCodegen::codegen_local_variable(const VariableDefn* defn) {
    lir::Type* type = to_lir_type(defn->get_type());
    lir::Local* local = lir::Local::create(
        m_cfg, 
        type, 
        defn->get_name(), 
        m_func
    );

    if (!defn->has_init())
        return local;

    if (m_mach.is_scalar(type)) {
        lir::Value* value = codegen_valued_expression(defn->get_init());
        assert(value);

        m_builder.build_store(value, local);
    } else {
        m_state.place = local;

        lir::Value* value = codegen_addressed_expression(defn->get_init());
        assert(value);

        if (value != local) {
            m_builder.build_call(getIntrinsicCopy(), {
                local,
                value,
                lir::Integer::get(m_cfg, lir::IntegerType::get(m_cfg, 64), m_mach.get_type_size(type) / 8),
            });
        }

        m_state.place = nullptr;
    }

    return local;
}
