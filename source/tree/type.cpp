#include "tree/decl.hpp"
#include "tree/root.hpp"
#include "tree/type.hpp"

#include <cassert>

using namespace stm;

const DeferredType* DeferredType::get(Root& root, const Context& context) {
    return root.context().get(context);
}

std::string DeferredType::to_string() const {
    std::string str = is_mut() ? "mut " : "";
    for (u32 p = 0; p != m_context.indirection; ++p)
        str += '*';

    return str + m_context.base;
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
        return "s8";
    case Kind::SInt16: 
        return "s16";
    case Kind::SInt32: 
        return "s32";
    case Kind::SInt64: 
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

bool BuiltinType::compare(const Type* other) const {
    assert(other && "other type cannot be null!");

    if (other->is_deferred())
        other = other->as_deferred()->get_resolved();

    //if (is_mut() != other->is_mut())
    //    return false;

    if (!other->is_builtin())
        return false;

    return kind() == other->as_builtin()->kind();
}

bool BuiltinType::can_cast(const Type* other, bool impl) const {
    assert(other && "other type cannot be null!");

    if (other->is_deferred())
        other = other->as_deferred()->get_resolved();

    if (is_mut() != other->is_mut())
        return false;

    if (impl) {
        if (!other->is_builtin())
            return false;

        if (is_float() && other->is_int())
            return false;
    
        return is_void() == other->is_void();
    } else {
        if (other->is_builtin())
            return is_void() == other->is_void();
        else if (other->is_pointer())
            return is_int();

        return false;
    }
}

std::string BuiltinType::to_string() const {
    return is_mut() ? "mut " : "" + std::string(get_name(kind()));
}

const FunctionType* FunctionType::get(Root& root, const Type *ret, 
                                      const std::vector<const Type*>& params) {
    return root.context().get(ret, params);
}

std::string FunctionType::to_string() const {
    std::string str = "(";

    for (u32 idx = 0, e = num_params(); idx != e; ++idx) {
        str += get_param_type(idx)->to_string();
        if (idx + 1 != e)
            str += ", ";
    }

    return str + ") -> " + get_return_type()->to_string();
}

const PointerType* PointerType::get(Root& root, const Type* pointee) {
    return root.context().get(pointee);
}

u32 PointerType::get_indirection() const {
    u32 indir = 1;
    if (get_pointee()->is_pointer())
        indir += get_pointee()->as_pointer()->get_indirection();

    return indir;
}

bool PointerType::compare(const Type* other) const {
    assert(other && "other type cannot be null!");

    if (other->is_deferred())
        other = other->as_deferred()->get_resolved();

    if (is_mut() != other->is_mut())
        return false;

    if (!other->is_pointer())
        return false;

    return get_pointee()->compare(other->as_pointer()->get_pointee());
}

bool PointerType::can_cast(const Type* other, bool impl) const {
    assert(other && "other type cannot be null!");

    if (other->is_deferred())
        other = other->as_deferred()->get_resolved();

    if (is_mut() != other->is_mut())
        return false;

    if (impl) {
        if (get_pointee()->is_void())
            return true;
        else if (other->is_pointer())
            return other->as_pointer()->get_pointee()->is_void();

        return false;
    } else return other->is_pointer() || other->is_int();
}

std::string PointerType::to_string() const {
    return is_mut() ? "mut *" : "*" + get_pointee()->to_string();
}

const StructType* StructType::get(Root& root, const std::string& name) {
    const Type* type = root.context().get(name);
    if (!type)
        return nullptr;

    auto structure = dynamic_cast<const StructType*>(type);
    if (!structure)
        return nullptr;

    return structure;
}

const StructType* StructType::create(Root& root, 
                                     const std::vector<const Type*>& fields, 
                                     const StructDecl* decl) {
    return root.context().create(fields, decl);
}

bool StructType::compare(const Type* other) const {
    assert(other && "other type cannot be null!");

    if (other->is_deferred())
        other = other->as_deferred()->get_resolved();

    if (is_mut() != other->is_mut())
        return false;

    if (!other->is_struct())
        return false;

    return get_decl()->get_name() == other->as_struct()->get_decl()->get_name();
}

std::string StructType::to_string() const {
    return is_mut() ? "mut " : get_decl()->get_name();
}

const EnumType* EnumType::get(Root& root, const std::string& name) {
    const Type* type = root.context().get(name);
    if (!type)
        return nullptr;

    auto enumeration = dynamic_cast<const EnumType*>(type);
    if (!enumeration)
        return nullptr;

    return enumeration;
}

const EnumType* EnumType::create(Root& root, const Type* underlying,
                                 const EnumDecl* decl) {
    return root.context().create(underlying, decl);
}

bool EnumType::compare(const Type* other) const {
    assert(other && "other type cannot be null!");

    if (other->is_deferred())
        other = other->as_deferred()->get_resolved();

    if (is_mut() != other->is_mut())
        return false;

    if (!other->is_enum())
        return false;

    return get_decl()->get_name() == other->as_enum()->get_decl()->get_name();
}

std::string EnumType::to_string() const {
    return is_mut() ? "mut " : "" + get_decl()->get_name();
}
