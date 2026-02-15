//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lace/tree/AST.h"
#include "lace/tree/Defn.h"
#include "lace/tree/Type.h"

#include <cassert>

using namespace lace;

bool QualType::compare(const QualType& other) const {
    return /*m_quals == other.m_quals &&*/ m_type->compare(other.get_type());
}

bool QualType::can_cast(const QualType& other, bool implicitly) const {
    // @Todo: reinvent this, but careful of (mut lval) <- (immut rval) failing.
    // In the above case, the type checker falls back to trying a cast, but 
    // fails due to differences in mutability. Needs more thorough context.
    
    //if (other.is_mut() && !is_mut())
    //    return false;

    return get_type()->can_cast(other.get_type(), implicitly);
}

std::string QualType::to_string() const {
    std::string res = "";

    if (is_mut())
        res += "mut ";

    return res + get_type()->to_string();
}

AliasType* AliasType::create(AST::Context& ctx, const QualType& underlying,
                             const AliasDefn* defn) {
    assert(defn && "definition cannot be null!");
    
    AliasType* type = new AliasType(underlying, defn);
    ctx.m_aliases.emplace(defn->get_name(), type);
    return type;
}

AliasType* AliasType::get(AST::Context& ctx, const std::string &name) {
    auto it = ctx.m_aliases.find(name);
    if (it != ctx.m_aliases.end())
        return it->second;

    return nullptr;
}

std::string AliasType::to_string() const {
    assert(m_defn && "type has no declaration set!");
    return m_defn->get_name();
}

bool AliasType::can_cast(const Type* other, bool implicitly) const {
    assert(other && "other type cannot be null!");
    return get_underlying().can_cast(other, implicitly);
}

ArrayType* ArrayType::get(AST::Context& ctx, const QualType& element, 
                           uint32_t size) {
    ArrayType* type = new ArrayType(element, size);
    ctx.m_arrays.push_back(type);
    return type;
}

bool ArrayType::compare(const Type* other) const {
    assert(other && "other type cannot be null!");

    if (!other->isClass(Class::Array))
        return false;

    auto AT = static_cast<const ArrayType*>(other);
    return get_size() == AT->get_size() 
        && get_element_type().compare(AT->get_element_type());
}

bool ArrayType::can_cast(const Type* other, bool implicitly) const {
    assert(other && "other type cannot be null!");

    // Can only cast [...]T -> *T.
    if (!other->isClass(Class::Pointer))
        return false;

    return get_element_type().can_cast(
        static_cast<const PointerType*>(other)->get_pointee());
}

BuiltinType* BuiltinType::get(AST::Context &ctx, Kind kind) {
    return ctx.m_builtins[kind];
}

std::string BuiltinType::to_string() const {
    switch (get_kind()) {
        case Void:      return "void";
        case Bool:      return "bool";
        case Char:      return "char";
        case Int8:      return "s8";
        case Int16:     return "s16";
        case Int32:     return "s32";
        case Int64:     return "s64";
        case UInt8:     return "u8";
        case UInt16:    return "u16";
        case UInt32:    return "u32";
        case UInt64:    return "u64";
        case Float32:   return "f32";
        case Float64:   return "f64";
    }
}

bool BuiltinType::compare(const Type* other) const {
    assert(other && "other type cannot be null!");
    return other->isClass(Class::Builtin) 
        && get_kind() == static_cast<const BuiltinType*>(other)->get_kind();
}

bool BuiltinType::can_cast(const Type* other, bool implicitly) const {
    assert(other && "other type cannot be null!");

    if (implicitly) {
        if (!other->isClass(Class::Builtin))
            return false;

        if (is_floating_point() && other->is_integer())
            return false;

        return is_void() == other->is_void();
    } else {
        if (other->isClass(Class::Builtin))
            return is_void() == other->is_void();

        if (other->isClass(Class::Pointer))
            return is_integer();

        return false;
    }
}

DeferredType* DeferredType::get(AST::Context& ctx, const std::string& name) {
    DeferredType* type = new DeferredType(name);
    ctx.m_deferred.push_back(type);
    return type;
}

EnumType* EnumType::create(AST::Context& ctx, const QualType& underlying, 
                           const EnumDefn* defn) {
    assert(defn && "definition cannot be null!");

    auto it = ctx.m_enums.find(defn->get_name());
    if (it != ctx.m_enums.end())
        return nullptr;

    EnumType* type = new EnumType(underlying, defn);
    ctx.m_enums.emplace(defn->get_name(), type);
    return type;
}

EnumType* EnumType::get(AST::Context& ctx, const std::string& name) {
    auto it = ctx.m_enums.find(name);
    if (it != ctx.m_enums.end())
        return it->second;

    return nullptr;
}

std::string EnumType::to_string() const {
    assert(m_defn && "type has no declaration set!");
    return m_defn->get_name();
}

bool EnumType::can_cast(const Type* other, bool implicitly) const {
    assert(other && "other type cannot be null!");
    return other->is_integer();
}

FunctionType* FunctionType::get(AST::Context& ctx, const QualType& ret, 
                                const Params& params) {
    FunctionType* type = new FunctionType(ret, params);
    ctx.m_functions.push_back(type);
    return type;
}

std::string FunctionType::to_string() const {
    std::string res = "(";
    for (uint32_t i = 0, e = num_params(); i != e; ++i) {
        res += get_param(i).to_string();
        if (i + 1 != e)
            res += ", ";
    }

    return res + ") -> " + get_return_type().to_string();
}

PointerType* PointerType::get(AST::Context& ctx, const QualType& pointee) {
    PointerType* type = new PointerType(pointee);
    ctx.m_pointers.push_back(type);
    return type;
}

bool PointerType::compare(const Type* other) const {
    assert(other && "other type cannot be null!");

    if (!other->isClass(Class::Pointer))
        return false;

    return get_pointee().compare(
        static_cast<const PointerType*>(other)->get_pointee());
}

bool PointerType::can_cast(const Type* other, bool implicitly) const {
    assert(other && "other type cannot be null!");

    if (implicitly) {
        // Can implicitly cast *void -> *T.
        if (get_pointee()->is_void())
            return true;

        // Cannot implicitly cast away pointer indirection.
        if (!other->isClass(Class::Pointer))
            return false;

        // Can implicitly cast *T -> *void.
        return static_cast<const PointerType*>(other)->get_pointee()->is_void();
    } else {
        // Can explicitly cast to other pointer types or integers.
        return other->isClass(Class::Pointer) || other->is_integer();
    }
}

StructType* StructType::create(AST::Context& ctx, const StructDefn *defn) {
    assert(defn && "definition cannot be null!");
    
    auto it = ctx.m_structs.find(defn->get_name());
    if (it != ctx.m_structs.end())
        return nullptr;

    StructType* type = new StructType(defn);
    ctx.m_structs.emplace(defn->get_name(), type);
    return type;
}

StructType* StructType::get(AST::Context& ctx, const std::string& name) {
    auto it = ctx.m_structs.find(name);
    if (it != ctx.m_structs.end())
        return it->second;

    return nullptr;
}

std::string StructType::to_string() const {
    assert(m_defn && "type has no declaration set!");
    return m_defn->get_name();
}
