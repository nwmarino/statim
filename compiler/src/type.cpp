#include "ast.hpp"
#include "type.hpp"

#include <cassert>

using namespace stm;

Type::id_t Type::it = 0;

const DeferredType* DeferredType::get(Root& root, const Context& context) {
    return root.get_context().get(context);
}

bool DeferredType::is_void() const {
    assert(pResolved && "deferred type unresolved");
    return pResolved->is_void();
}

bool DeferredType::is_bool() const {
    assert(pResolved && "deferred type unresolved");
    return pResolved->is_bool();
}

bool DeferredType::is_int() const {
    assert(pResolved && "deferred type unresolved");
    return pResolved->is_int();
}

bool DeferredType::is_signed_int() const {
    assert(pResolved && "deferred type unresolved");
    return pResolved->is_signed_int();
}

bool DeferredType::is_unsigned_int() const {
    assert(pResolved && "deferred type unresolved");
    return pResolved->is_unsigned_int();
}

bool DeferredType::is_float() const {
    assert(pResolved && "deferred type unresolved");
    return pResolved->is_float();
}

const PointerType* DeferredType::as_pointer() const {
    assert(pResolved && "deferred type unresolved");
    return static_cast<const PointerType*>(pResolved);
}

const StructType* DeferredType::as_struct() const {
    assert(pResolved && "deferred type unresolved");
    return static_cast<const StructType*>(pResolved);
}

const EnumType* DeferredType::as_enum() const {
    assert(pResolved && "deferred type unresolved");
    return static_cast<const EnumType*>(pResolved);
}

bool DeferredType::can_cast(const Type* other, bool impl) const {
    assert(pResolved && "deferred type unresolved");
    return pResolved->can_cast(other, impl);
}

std::string DeferredType::to_string() const {
    std::string str = "";
    for (u32 p = 0; p != context.indirection; ++p)
        str += '*';

    return str + context.base;
}

std::string BuiltinType::get_name(Kind kind) {
    switch (kind) {
    case Kind::Void: 
        return "void";
    case Kind::Bool: 
        return "bool";
    case Kind::Char: 
        return "char";
    case Kind::SInt8: 
        return "i8";
    case Kind::SInt16: 
        return "i16";
    case Kind::SInt32: 
        return "i32";
    case Kind::SInt64: 
        return "i64";
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

const BuiltinType* BuiltinType::get(Root& root, Kind kind) {
    switch (kind) {
    case Kind::Void: 
        return root.get_void_type();
    case Kind::Bool: 
        return root.get_bool_type();
    case Kind::Char: 
        return root.get_char_type();
    case Kind::SInt8: 
        return root.get_si8_type();
    case Kind::SInt16: 
        return root.get_si16_type();
    case Kind::SInt32: 
        return root.get_si32_type();
    case Kind::SInt64: 
        return root.get_si64_type();
    case Kind::UInt8: 
        return root.get_ui8_type();
    case Kind::UInt16: 
        return root.get_ui16_type();
    case Kind::UInt32: 
        return root.get_ui32_type();
    case Kind::UInt64: 
        return root.get_ui64_type();
    case Kind::Float32: 
        return root.get_fp32_type();
    case Kind::Float64: 
        return root.get_fp64_type();
    }
}

bool BuiltinType::is_int() const {
    switch (kind) {
    case Kind::Bool:
    case Kind::Char:
    case Kind::SInt8:
    case Kind::SInt16:
    case Kind::SInt32:
    case Kind::SInt64:
    case Kind::UInt8:
    case Kind::UInt16:
    case Kind::UInt32:
    case Kind::UInt64:
        return true;
    default:
        return false;
    }
}

bool BuiltinType::is_signed_int() const {
    switch (kind) {
    case Kind::Bool:
    case Kind::Char:
    case Kind::SInt8:
    case Kind::SInt16:
    case Kind::SInt32:
    case Kind::SInt64:
        return true;
    default:
        return false;
    }
}

bool BuiltinType::is_unsigned_int() const {
    switch (kind) {
    case Kind::UInt8:
    case Kind::UInt16:
    case Kind::UInt32:
    case Kind::UInt64:
        return true;
    default:
        return false;
    }
}

bool BuiltinType::is_float() const {
    switch (kind) {
    case Kind::Float32:
    case Kind::Float64:
        return true;
    default:
        return false;
    }
}

bool BuiltinType::can_cast(const Type* other, bool impl) const {
    assert(other && "other type cannot be null");

    if (impl) {
        if (auto deferred = dynamic_cast<const DeferredType*>(other))
            return can_cast(deferred->get_resolved(), true);

        auto builtin = dynamic_cast<const BuiltinType*>(other);
        if (!builtin)
            return false;

        if (is_float() && other->is_int())
            return false;
    
        return is_void() == other->is_void();
    } else {
        if (auto builtin = dynamic_cast<const BuiltinType*>(other))
            return is_void() == other->is_void();
        else if (auto ptr = dynamic_cast<const PointerType*>(other))
            return is_int();
        else if (auto deferred = dynamic_cast<const DeferredType*>(other))
            return can_cast(deferred->get_resolved(), false);

        return false;
    }
}

std::string BuiltinType::to_string() const {
    return std::string(get_name(this->kind));
}

FunctionType::FunctionType(const Type* pReturn, const std::vector<const Type*>& params)
    : pReturn(pReturn), params(params) {};

const FunctionType* FunctionType::get(
        Root& root, 
        const Type *pReturn, 
        const std::vector<const Type*>& params) {
    return root.get_context().get(pReturn, params);
}

std::string FunctionType::to_string() const {
    std::string str = "(";

    for (u32 idx = 0, e = params.size(); idx != e; ++idx)
        str += params[idx]->to_string() + (idx + 1 != e ? ", " : "");

    return str + ") -> " + this->pReturn->to_string();
}

const PointerType* PointerType::get(Root& root, const Type* pPointee) {
    return root.get_context().get(pPointee);
}

u32 PointerType::get_indirection() const {
    u32 indir = 1;
    if (auto* ptr = dynamic_cast<const PointerType*>(this->pPointee))
        indir += ptr->get_indirection();

    return indir;
}

bool PointerType::can_cast(const Type* other, bool impl) const {
    assert(other && "other type cannot be null");

    if (other->is_deferred())
        return can_cast(other->as_deferred()->get_resolved(), impl);

    if (impl) {
        if (pPointee->is_void())
            return true;
        else if (other->is_pointer())
            return other->as_pointer()->get_pointee()->is_void();

        return false;
    } else return other->is_pointer() || other->is_int();
}

std::string PointerType::to_string() const {
    return "*" + this->pPointee->to_string();
}

const StructType* StructType::get(Root& root, const std::string& name) {
    const Type* type = root.get_context().get(name);
    if (!type)
        return nullptr;

    const StructType* st = dynamic_cast<const StructType*>(type);
    if (!st)
        return nullptr;

    return st;
}

const StructType* StructType::create(
        Root& root, 
        const std::vector<const Type*>& fields, 
        const StructDecl* decl) {
    return root.get_context().create(fields, decl);
}

std::string StructType::to_string() const {
    return m_decl->get_name();
}

const EnumType* EnumType::get(Root& root, const std::string& name) {
    const Type* type = root.get_context().get(name);
    if (!type)
        return nullptr;

    const EnumType* et = dynamic_cast<const EnumType*>(type);
    if (!et)
        return nullptr;

    return et;
}

const EnumType* EnumType::create(
        Root& root,
        const Type* underlying,
        const EnumDecl* decl) {
    return root.get_context().create(underlying, decl);
}

std::string EnumType::to_string() const {
    return m_decl->get_name();
}
