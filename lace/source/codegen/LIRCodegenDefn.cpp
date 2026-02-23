//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/core/Diagnostics.h"
#include "lace/tree/Defn.h"
#include "lace/tree/LIRCodegen.h"
#include "lace/tree/Type.h"

#include "lir/graph/Constant.h"
#include "lir/graph/Function.h"
#include "lir/graph/BasicBlock.h"
#include "lir/graph/Type.h"

using namespace lace;

void LIRCodegen::codegen_initial_definition(const Defn* defn) {
    if (auto space = dynamic_cast<const SpaceDefn*>(defn)) {
        codegen_initial_namespace(space);
    } else if (auto func = dynamic_cast<const FunctionDefn*>(defn)) {
        codegen_initial_function(func);
    } else if (auto structure = dynamic_cast<const StructDefn*>(defn)) {
        codegen_initial_structure(structure);
    } else if (auto var = dynamic_cast<const VariableDefn*>(defn)) {
        codegen_initial_global(var);
    }
}

void LIRCodegen::codegen_lowered_definition(const Defn* defn) {
    if (auto space = dynamic_cast<const SpaceDefn*>(defn)) {
        codegen_lowered_namespace(space);
    } else if (auto func = dynamic_cast<const FunctionDefn*>(defn)) {
        codegen_lowered_function(func);
    } else if (auto structure = dynamic_cast<const StructDefn*>(defn)) {
        codegen_lowered_structure(structure);
    } else if (auto var = dynamic_cast<const VariableDefn*>(defn)) {
        codegen_lowered_global(var);
    }
}

void LIRCodegen::codegen_initial_namespace(const SpaceDefn* defn) {
    enter_namespace(defn);

    for (NamedDefn* named_defn : defn->defns()) {
        if (named_defn->origin() != m_ast && !named_defn->has_rune(Rune::Kind::Public))
            continue;

        codegen_initial_definition(named_defn);
    }

    exit_namespace();
}

void LIRCodegen::codegen_lowered_namespace(const SpaceDefn* defn) {
    enter_namespace(defn);

    for (NamedDefn* named_defn : defn->defns()) {
        if (named_defn->origin() != m_ast)
            continue;

        codegen_lowered_definition(named_defn);
    }

    exit_namespace();
}

lir::Function* LIRCodegen::codegen_initial_function(const FunctionDefn* defn) {
    assert(!m_funcs.contains(defn));

    auto linkage = lir::Function::LinkageType::Private;
    if (defn->has_rune(Rune::Kind::Public))
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

    for (const ParameterDefn* param : defn->params()) {
        auto trait = lir::Parameter::Trait::None;

        std::string name = param->name();
        if (name == "_") {
            // Unnamed parameters i.e. '_' should be cleared at this point. 
            name.clear();
        }

        lir::Type* type = to_lir_type(param->type());
        if (!m_mach.is_scalar(type)) {
            // Parameter type is an aggregate, so it should be passed "by value" via a hidden ptr.
            type = lir::PointerType::get(m_cfg, type);
            trait = lir::Parameter::Trait::Byval;
        }

        params.push_back(lir::Parameter::create(type, name, trait));
    }

    lir::FunctionType* func_type = dynamic_cast<lir::FunctionType*>(to_lir_type(defn->type()));
    assert(func_type);

    lir::Function* func = lir::Function::create(
        m_cfg, 
        linkage, 
        func_type,
        get_namespace_prefix() + defn->name(), 
        params
    );

    m_funcs.emplace(defn, func);
    return func;
}

lir::Function* LIRCodegen::codegen_lowered_function(const FunctionDefn* defn) {
    assert(m_funcs.contains(defn) && "function not lowered!");

    lir::Function* func = m_funcs.at(defn);

    // Assert that the function was added to the parent graph.
    assert(m_cfg.get_function(func->get_name()));

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

    codegen_statement(defn->body());

    if (!m_builder.get_insert()->terminates()) {
        if (!m_func->get_type()->has_result()) {
            m_builder.build_ret();
        } else {
            log::warn("non-void function does not always return a value", 
                log::Span(m_cfg.get_filename(), defn->span()));
        }
    }

    m_func = nullptr;
    m_builder.clear_insert();
    return func;
}

lir::Global* LIRCodegen::codegen_initial_global(const VariableDefn* defn) {
    assert(!m_globals.contains(defn));

    auto linkage = lir::Global::LinkageType::Private;
    if (defn->has_rune(Rune::Kind::Public))
        linkage = lir::Global::LinkageType::Public;

    lir::Global* global = lir::Global::create(
        m_cfg, 
        to_lir_type(defn->type()), 
        linkage, 
        defn->name(),
        true // mutable
    );

    m_globals.emplace(defn, global);
    return global;
}

lir::Global* LIRCodegen::codegen_lowered_global(const VariableDefn* defn) {
    assert(m_globals.contains(defn) && "function not lowered!");

    lir::Global* global = m_globals.at(defn);

    // Assert that the function was added to the parent graph.
    assert(m_cfg.get_global(global->get_name()));

    if (!defn->has_init())
        return global;

    lir::Value* value = codegen_valued_expression(defn->init());
    assert(value);
    
    lir::Constant* init = dynamic_cast<lir::Constant*>(value);
    assert(init && "global is not initialized with a constant!");

    global->set_initializer(init);
    return global;
}

lir::StructType* LIRCodegen::codegen_initial_structure(const StructDefn* defn) {
    assert(!m_structs.contains(defn));

    lir::StructType* structure = lir::StructType::create(m_cfg, defn->name(), {});

    m_structs.emplace(defn, structure);
    return structure;
}

lir::StructType* LIRCodegen::codegen_lowered_structure(const StructDefn* defn) {
    assert(m_structs.contains(defn));
    
    lir::StructType* type = m_structs.at(defn);
    
    assert(lir::StructType::get(m_cfg, type->get_name()));

    for (const FieldDefn* field : defn->fields())
        type->append_field(to_lir_type(field->type()));

    return type;
}

lir::Local* LIRCodegen::codegen_local_variable(const VariableDefn* defn) {
    lir::Type* type = to_lir_type(defn->type());
    lir::Local* local = lir::Local::create(
        m_cfg, 
        type, 
        defn->name(), 
        m_func
    );

    if (!defn->has_init())
        return local;

    if (m_mach.is_scalar(type)) {
        lir::Value* value = codegen_valued_expression(defn->init());
        assert(value);

        m_builder.build_store(value, local);
    } else {
        m_state.place = local;

        lir::Value* value = codegen_addressed_expression(defn->init());
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
