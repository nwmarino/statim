//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/tree/AST.h"
#include "lace/tree/Defn.h"
#include "lace/tree/Type.h"

#include <cassert>

using namespace lace;

//>==---------------------------------------------------------------------------
//                          AliasType Implementation
//>==---------------------------------------------------------------------------

AliasType* AliasType::create(AST::Context& ctx, Type* aliased, AliasDefn* defn) {
    assert(defn && "defn cannot be null!");
    
    AliasType* type = new AliasType(aliased, defn);
    assert(type);

    ctx.m_aliases.emplace(defn->name(), type);
    return type;
}

AliasType* AliasType::get(AST::Context& ctx, const std::string &name) {
    auto it = ctx.m_aliases.find(name);
    if (it != ctx.m_aliases.end())
        return it->second;

    return nullptr;
}

std::string AliasType::string() const {
    assert(m_defn && "type has no declaration set!");

    return m_defn->name();
}

bool AliasType::can_cast(const Type* other, bool implicit) const {
    assert(other && "other type cannot be null!");

    return m_aliased->can_cast(other, implicit);
}

//>==---------------------------------------------------------------------------
//                          BuiltinType Implementation
//>==---------------------------------------------------------------------------

BuiltinType* BuiltinType::get(AST::Context &ctx, Kind kind) {
    return ctx.m_builtins[static_cast<uint32_t>(kind)];
}

std::string BuiltinType::string() const {
    switch (m_kind) 
    {
    case Kind::Void:      
        return "void";
    case Kind::Bool:      
        return "bool";
    case Kind::Char:      
        return "char";
    case Kind::Int8:      
        return "s8";
    case Kind::Int16:     
        return "s16";
    case Kind::Int32:     
        return "s32";
    case Kind::Int64:     
        return "s64";
    case Kind::UInt8:     
        return "u8";
    case Kind::UInt16:    
        return "u16";
    case Kind::UInt32:    
        return "u32";
    case Kind::UInt64:    
        return "u64";
    case Kind::Float32:   
        return "f32";
    case Kind::Float64:   
        return "f64";
    }
}

bool BuiltinType::compare(const Type* other) const {
    assert(other && "other type cannot be null!");

    auto BT = dynamic_cast<const BuiltinType*>(other);
    if (!BT)
        return false;

    return m_kind == BT->kind();
}

bool BuiltinType::can_cast(const Type* other, bool implicit) const {
    assert(other && "other type cannot be null!");

    if (implicit) {
        if (!dynamic_cast<const BuiltinType*>(other))
            return false;

        if (is_floating_point() && other->is_integer())
            return false;

        return is_void() == other->is_void();
    } else {
        if (dynamic_cast<const BuiltinType*>(other))
            return is_void() == other->is_void();

        // int -> pointer is allowed, explicitly.
        if (dynamic_cast<const PointerType*>(other))
            return is_integer();

        return false;
    }
}

DeferredType* DeferredType::get(AST::Context& ctx, const std::string& name) {
    DeferredType* type = new DeferredType(name);
    assert(type);

    ctx.m_deferred.push_back(type);
    return type;
}

//>==---------------------------------------------------------------------------
//                          EnumType Implementation
//>==---------------------------------------------------------------------------

EnumType* EnumType::create(AST::Context& ctx, Type* underlying, EnumDefn* defn) {
    assert(defn && "definition cannot be null!");

    auto it = ctx.m_enums.find(defn->name());
    if (it != ctx.m_enums.end())
        return nullptr;

    EnumType* type = new EnumType(underlying, defn);
    assert(type);

    ctx.m_enums.emplace(defn->name(), type);
    return type;
}

EnumType* EnumType::get(AST::Context& ctx, const std::string& name) {
    auto it = ctx.m_enums.find(name);
    if (it != ctx.m_enums.end())
        return it->second;

    return nullptr;
}

std::string EnumType::string() const {
    assert(m_defn && "type has no declaration set!");

    return m_defn->name();
}

bool EnumType::can_cast(const Type* other, bool implicit) const {
    assert(other && "other type cannot be null!");
    
    return other->is_integer();
}

//>==---------------------------------------------------------------------------
//                          FunctionType Implementation
//>==---------------------------------------------------------------------------

FunctionType* FunctionType::get(AST::Context& ctx, Type* result, 
                                const std::vector<Type*>& params) {
    FunctionType* type = new FunctionType(result, params);
    assert(type);
    
    ctx.m_functions.push_back(type);
    return type;
}

std::string FunctionType::string() const {
    std::string res = "(";
    for (uint32_t i = 0, e = num_params(); i != e; ++i) {
        res += m_params[i]->string();
        if (i + 1 != e)
            res += ", ";
    }

    return std::format("{}) -> {}", res, m_result->string());
}

//>==---------------------------------------------------------------------------
//                          PointerType Implementation
//>==---------------------------------------------------------------------------

PointerType* PointerType::get(AST::Context& ctx, Type* pointee) {
    PointerType* type = new PointerType(pointee);
    assert(type);

    ctx.m_pointers.push_back(type);
    return type;
}

bool PointerType::compare(const Type* other) const {
    assert(other && "other type cannot be null!");

    auto PT = dynamic_cast<const PointerType*>(other);
    if (!PT)
        return false;

    return m_pointee->compare(PT->pointee());
}

bool PointerType::can_cast(const Type* other, bool implicit) const {
    assert(other && "other type cannot be null!");

    if (implicit) {
        // Can implicitly cast *void -> *T.
        if (m_pointee->is_void())
            return true;

        // Cannot implicitly cast away pointer indirection.
        auto PT = dynamic_cast<const PointerType*>(other);
        if (!PT)
            return false;

        // Can implicitly cast *T -> *void.
        return PT->pointee()->is_void();
    } else {
        // Can explicitly cast to other pointer types or integers.
        return other->is_integer() || dynamic_cast<const PointerType*>(other);
    }
}

//>==---------------------------------------------------------------------------
//                          StructType Implementation
//>==---------------------------------------------------------------------------

StructType* StructType::create(AST::Context& ctx, StructDefn* defn) {
    assert(defn && "definition cannot be null!");
    
    auto it = ctx.m_structs.find(defn->name());
    if (it != ctx.m_structs.end())
        return nullptr;

    StructType* type = new StructType(defn);
    assert(type);

    ctx.m_structs.emplace(defn->name(), type);
    return type;
}

StructType* StructType::get(AST::Context& ctx, const std::string& name) {
    auto it = ctx.m_structs.find(name);
    if (it != ctx.m_structs.end())
        return it->second;

    return nullptr;
}

std::string StructType::string() const {
    assert(m_defn && "type has no declaration set!");
    return m_defn->name();
}
