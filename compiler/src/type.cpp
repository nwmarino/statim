#include "ast.hpp"
#include "type.hpp"

using namespace stm;

Type::id_t Type::it = 0;

const DeferredType* DeferredType::get(Root& root, const Context& context) {
    return root.get_context().get(context);
}

std::string DeferredType::to_string() const {
    std::string str = "";
    for (u32 p = 0; p != context.indirection; ++p)
        str += '*';

    return str + context.base;
}

std::string BuiltinType::get_name(Kind kind) {
    switch (kind) {
        case Kind::Void: return "void";
        case Kind::Bool: return "bool";
        case Kind::Char: return "char";
        case Kind::SInt8: return "i8";
        case Kind::SInt16: return "i16";
        case Kind::SInt32: return "i32";
        case Kind::SInt64: return "i64";
        case Kind::UInt8: return "u8";
        case Kind::UInt16: return "u16";
        case Kind::UInt32: return "u32";
        case Kind::UInt64: return "u64";
        case Kind::Float32: return "f32";
        case Kind::Float64: return "f64";
    }
}

const BuiltinType* BuiltinType::get(Root& root, Kind kind) {
    switch (kind) {
        case Kind::Void: return root.get_void_type();
        case Kind::Bool: return root.get_bool_type();
        case Kind::Char: return root.get_char_type();
        case Kind::SInt8: return root.get_si8_type();
        case Kind::SInt16: return root.get_si16_type();
        case Kind::SInt32: return root.get_si32_type();
        case Kind::SInt64: return root.get_si64_type();
        case Kind::UInt8: return root.get_ui8_type();
        case Kind::UInt16: return root.get_ui16_type();
        case Kind::UInt32: return root.get_ui32_type();
        case Kind::UInt64: return root.get_ui64_type();
        case Kind::Float32: return root.get_fp32_type();
        case Kind::Float64: return root.get_fp64_type();
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

    for (u32 idx = 0, e = this->params.size(); idx != e; ++idx)
        str += this->params[idx]->to_string() + (idx + 1 != e ? ", " : "");

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

std::string PointerType::to_string() const {
    return "*" + this->pPointee->to_string();
}
