#include "core/logger.hpp"
#include "siir/cfg.hpp"
#include "siir/type.hpp"

using namespace stm;
using namespace stm::siir;

u32 Type::s_id_iter = 0;

const Type* Type::get_i1_type(CFG& cfg) {
    return cfg.m_types_ints[IntegerType::TY_Int1];
}

const Type* Type::get_i8_type(CFG& cfg) {
    return cfg.m_types_ints[IntegerType::TY_Int8];
}

const Type* Type::get_i16_type(CFG& cfg) {
    return cfg.m_types_ints[IntegerType::TY_Int16];
}

const Type* Type::get_i32_type(CFG& cfg) {
    return cfg.m_types_ints[IntegerType::TY_Int32];
}

const Type* Type::get_i64_type(CFG& cfg) {
    return cfg.m_types_ints[IntegerType::TY_Int64];
}

const Type* Type::get_f32_type(CFG& cfg) {
    return cfg.m_types_floats[FloatType::TY_Float32];
}

const Type* Type::get_f64_type(CFG& cfg) {
    return cfg.m_types_floats[FloatType::TY_Float64];
}

static const IntegerType* get(CFG& cfg, u32 width) {
    switch (width) {
    case 1:
        return static_cast<const IntegerType*>(Type::get_i1_type(cfg));
    case 8:
        return static_cast<const IntegerType*>(Type::get_i8_type(cfg));
    case 16:
        return static_cast<const IntegerType*>(Type::get_i16_type(cfg));
    case 32:
        return static_cast<const IntegerType*>(Type::get_i32_type(cfg));
    case 64:
        return static_cast<const IntegerType*>(Type::get_i64_type(cfg));
    }

    assert(false && "uncompatible integer type bit width");
}

std::string IntegerType::to_string() const {
    switch (m_kind) {
    case TY_Int1:
        return "i1";
    case TY_Int8:
        return "i8";
    case TY_Int16:
        return "i16";
    case TY_Int32:
        return "i32";
    case TY_Int64:
        return "i64";
    }
}

const FloatType* FloatType::get(CFG& cfg, u32 width) {
    switch (width) {
    case 32:
        return static_cast<const FloatType*>(Type::get_f32_type(cfg));
    case 64:
        return static_cast<const FloatType*>(Type::get_f64_type(cfg));
    }

    assert(false && "uncompatible floating type bit width");
}

std::string FloatType::to_string() const {
    switch (m_kind) {
    case TY_Float32:
        return "f32";
    case TY_Float64:
        return "f64";
    }
}

const ArrayType* ArrayType::get(CFG& cfg, const Type* element, u32 size) {
    //auto element_it = cfg.m_types_arrays.find(element);
    //if (element_it != cfg.m_types_arrays.end()) {
    //    auto size_it = element_it->second.find(size);
    //    if (size_it != element_it->second.end())
    //        return size_it->second;
    //}

    ArrayType* type = new ArrayType(element, size);
    //std::pair<u32, ArrayType*> pair = { size, type };
    //cfg.m_types_arrays.emplace(element, pair);
    return type;
}

std::string ArrayType::to_string() const {
    return '[' + std::to_string(m_size) + ']' + m_element->to_string();
}

const FunctionType* 
FunctionType::get(CFG& cfg, const std::vector<const Type*>& args, 
                  const Type* ret) {
    FunctionType* type = new FunctionType(args, ret);
    cfg.m_types_fns.push_back(type);
    return type;
}

std::string FunctionType::to_string() const {
    std::string str = "(";
    for (u32 idx = 0, e = m_args.size(); idx != e; ++idx) {
        str += m_args[idx]->to_string();
        if (idx + 1 != e)
            str += ", ";
    }

    str += ')';
    if (m_ret)
        str += " -> " + m_ret->to_string();

    return str;
}

const PointerType* PointerType::get(CFG& cfg, const Type* pointee) {
    auto it = cfg.m_types_ptrs.find(pointee);
    if (it != cfg.m_types_ptrs.end())
        return it->second;

    PointerType* type = new PointerType(pointee);
    cfg.m_types_ptrs.emplace(pointee, type);
    return type;
}

std::string PointerType::to_string() const {
    std::string str = "*";

    if (m_pointee)
        str += m_pointee->to_string();
    else
        str += "void";

    return str;
}

StructType* StructType::get(CFG& cfg, const std::string& name) {
    auto it = cfg.m_types_structs.find(name);
    if (it != cfg.m_types_structs.end())
        return it->second;

    return nullptr;
}

StructType* 
StructType::create(CFG& cfg, const std::string& name,
                   const std::vector<const Type*> &fields) {
    assert(!get(cfg, name) && 
        "struct type with name already exists");

    StructType* type = new StructType(name, fields);
    assert(type);
    cfg.m_types_structs.emplace(name, type);
    return type;
}

std::string StructType::to_string() const {
    return m_name;
}
