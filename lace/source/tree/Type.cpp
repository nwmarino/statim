//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lace/tree/AST.h"
#include "lace/tree/Defn.h"
#include "lace/tree/Type.h"

#include <cassert>

using namespace lace;

//>==-----------------------------------------------------------------------------------------------
//                                      QualType Implementation
//>==-----------------------------------------------------------------------------------------------

bool QualType::compare(const QualType& other) const {
    return /* m_quals == other.m_quals && */ m_type->compare(other.getType());
}

bool QualType::canCast(const QualType& other, bool implicitly) const {
    // @Todo: reinvent this, but careful of (mut lval) <- (immut rval) failing.
    // In the above case, the type checker falls back to trying a cast, but 
    // fails due to differences in mutability. Needs more thorough context.
    
    //if (other.is_mut() && !is_mut())
    //    return false;

    return m_type->canCast(other.getType(), implicitly);
}

std::string QualType::string() const {
    std::string res = "";

    if (isMut())
        res += "mut ";

    return res + m_type->string();
}

//>==-----------------------------------------------------------------------------------------------
//                                      AliasType Implementation
//>==-----------------------------------------------------------------------------------------------

AliasType* AliasType::create(AST::Context& ctx, const QualType& underlying, const AliasDefn* defn) {
    assert(defn && "defn cannot be null!");
    
    AliasType* type = new AliasType(underlying, defn);
    assert(type);

    ctx.m_aliases.emplace(defn->get_name(), type);
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

    return m_defn->get_name();
}

Result AliasType::canCast(const Type* other, bool implicitly) const {
    assert(other && "other type cannot be null!");

    return m_underlying.canCast(other, implicitly);
}

//>==-----------------------------------------------------------------------------------------------
//                                      ArrayType Implementation
//>==-----------------------------------------------------------------------------------------------

ArrayType* ArrayType::get(AST::Context& ctx, const QualType& element, uint32_t size) {
    ArrayType* type = new ArrayType(element, size);
    assert(type);

    ctx.m_arrays.push_back(type);
    return type;
}

Result ArrayType::compare(const Type* other) const {
    assert(other && "other type cannot be null!");

    if (!other->isClass(Class::Array))
        return false;

    auto array_type = static_cast<const ArrayType*>(other);
    return m_size == array_type->size() && m_element.compare(array_type->element());
}

Result ArrayType::canCast(const Type* other, bool implicitly) const {
    assert(other && "other type cannot be null!");

    // Can only cast [...]T -> *T.
    if (!other->isClass(Class::Pointer))
        return false;

    return m_element.canCast(static_cast<const PointerType*>(other)->pointee());
}

//>==-----------------------------------------------------------------------------------------------
//                                      BuiltinType Implementation
//>==-----------------------------------------------------------------------------------------------

BuiltinType* BuiltinType::get(AST::Context &ctx, Kind kind) {
    return ctx.m_builtins[static_cast<uint32_t>(kind)];
}

std::string BuiltinType::string() const {
    switch (m_kind) {
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

Result BuiltinType::compare(const Type* other) const {
    assert(other && "other type cannot be null!");

    if (!other->isClass(Class::Builtin))
        return false;

    return m_kind == static_cast<const BuiltinType*>(other)->kind();
}

Result BuiltinType::canCast(const Type* other, bool implicitly) const {
    assert(other && "other type cannot be null!");

    if (implicitly) {
        if (!other->isClass(Class::Builtin))
            return false;

        if (isFloatingPoint() && other->isInteger())
            return false;

        return isVoid() == other->isVoid();
    } else {
        if (other->isClass(Class::Builtin))
            return isVoid() == other->isVoid();

        if (other->isClass(Class::Pointer))
            return isInteger();

        return false;
    }
}

DeferredType* DeferredType::get(AST::Context& ctx, const std::string& name) {
    DeferredType* type = new DeferredType(name);
    assert(type);

    ctx.m_deferred.push_back(type);
    return type;
}

//>==-----------------------------------------------------------------------------------------------
//                                      EnumType Implementation
//>==-----------------------------------------------------------------------------------------------

EnumType* EnumType::create(AST::Context& ctx, const QualType& underlying, const EnumDefn* defn) {
    assert(defn && "definition cannot be null!");

    auto it = ctx.m_enums.find(defn->get_name());
    if (it != ctx.m_enums.end())
        return nullptr;

    EnumType* type = new EnumType(underlying, defn);
    assert(type);

    ctx.m_enums.emplace(defn->get_name(), type);
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

    return m_defn->get_name();
}

Result EnumType::canCast(const Type* other, bool implicitly) const {
    assert(other && "other type cannot be null!");
    
    return other->isInteger();
}

//>==-----------------------------------------------------------------------------------------------
//                                      FunctionType Implementation
//>==-----------------------------------------------------------------------------------------------

FunctionType* FunctionType::get(AST::Context& ctx, const QualType& ret, 
                                const std::vector<QualType>& params) {
    FunctionType* type = new FunctionType(ret, params);
    assert(type);
    
    ctx.m_functions.push_back(type);
    return type;
}

std::string FunctionType::string() const {
    std::string res = "(";
    for (uint32_t i = 0, e = numParams(); i != e; ++i) {
        res += m_params[i].string();
        if (i + 1 != e)
            res += ", ";
    }

    return std::format("{}) -> {}", res, m_result.string());
}

//>==-----------------------------------------------------------------------------------------------
//                                      PointerType Implementation
//>==-----------------------------------------------------------------------------------------------

PointerType* PointerType::get(AST::Context& ctx, const QualType& pointee) {
    PointerType* type = new PointerType(pointee);
    assert(type);

    ctx.m_pointers.push_back(type);
    return type;
}

Result PointerType::compare(const Type* other) const {
    assert(other && "other type cannot be null!");

    if (!other->isClass(Class::Pointer))
        return false;

    return m_pointee.compare(static_cast<const PointerType*>(other)->pointee());
}

Result PointerType::canCast(const Type* other, bool implicitly) const {
    assert(other && "other type cannot be null!");

    if (implicitly) {
        // Can implicitly cast *void -> *T.
        if (m_pointee->isVoid())
            return true;

        // Cannot implicitly cast away pointer indirection.
        if (!other->isClass(Class::Pointer))
            return false;

        // Can implicitly cast *T -> *void.
        return static_cast<const PointerType*>(other)->pointee()->isVoid();
    } else {
        // Can explicitly cast to other pointer types or integers.
        return other->isClass(Class::Pointer) || other->isInteger();
    }
}

//>==-----------------------------------------------------------------------------------------------
//                                      StructType Implementation
//>==-----------------------------------------------------------------------------------------------

StructType* StructType::create(AST::Context& ctx, const StructDefn* defn) {
    assert(defn && "definition cannot be null!");
    
    auto it = ctx.m_structs.find(defn->get_name());
    if (it != ctx.m_structs.end())
        return nullptr;

    StructType* type = new StructType(defn);
    assert(type);

    ctx.m_structs.emplace(defn->get_name(), type);
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
    
    return m_defn->get_name();
}
