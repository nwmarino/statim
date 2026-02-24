//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lir/graph/CFG.h"
#include "lir/graph/Type.h"

using namespace lir;

//>==---------------------------------------------------------------------------
//                          Type Implementation
//>==---------------------------------------------------------------------------

uint32_t Type::s_id = 0;

VoidType *Type::get_void(CFG &cfg) {
    return cfg.m_types.void_type;
}

IntegerType *Type::get_i8(CFG &cfg) {
    return cfg.m_types.ints[8];
}

IntegerType *Type::get_i16(CFG &cfg) {
    return cfg.m_types.ints[16];
}

IntegerType *Type::get_i32(CFG &cfg) {
    return cfg.m_types.ints[32];
}

IntegerType *Type::get_i64(CFG &cfg) {
    return cfg.m_types.ints[64];
}

FloatType *Type::get_f32(CFG &cfg) {
    return cfg.m_types.floats[32];
}

FloatType *Type::get_f64(CFG &cfg) {
    return cfg.m_types.floats[64];
}

//>==---------------------------------------------------------------------------
//                          VoidType Implementation
//>==---------------------------------------------------------------------------

VoidType *VoidType::get(CFG &cfg) {
    return cfg.m_types.void_type;
}

//>==---------------------------------------------------------------------------
//                          IntegerType Implementation
//>==---------------------------------------------------------------------------

IntegerType *IntegerType::get(CFG &cfg, uint32_t width) {
    switch (width) {
        case 8:
            return static_cast<IntegerType*>(Type::get_i8(cfg));
        case 16:
            return static_cast<IntegerType*>(Type::get_i16(cfg));
        case 32:
            return static_cast<IntegerType*>(Type::get_i32(cfg));
        case 64:
            return static_cast<IntegerType*>(Type::get_i64(cfg));
    }

    assert(false && "unsupported integer bit width!");
}

std::string IntegerType::to_string() const {
    switch (m_width) {
        case 8:
            return "i8";
        case 16:
            return "i16";
        case 32:
            return "i32";
        case 64:
            return "i64";
    }

    assert(false && "unsupported integer bit width!");
}

//>==---------------------------------------------------------------------------
//                          FloatType Implementation
//>==---------------------------------------------------------------------------

FloatType *FloatType::get(CFG &cfg, uint32_t width) {
    switch (width) {
        case 32:
            return static_cast<FloatType*>(Type::get_f32(cfg));
        case 64:
            return static_cast<FloatType*>(Type::get_f64(cfg));
    }

    assert(false && "unsupported float bit width!");
}

std::string FloatType::to_string() const {
    switch (m_width) {
        case 32:
            return "f32";
        case 64:
            return "f64";
    }

    assert(false && "unsupported float bit width!");
}

//>==---------------------------------------------------------------------------
//                          ArrayType Implementation
//>==---------------------------------------------------------------------------

ArrayType *ArrayType::get(CFG &cfg, Type *element, uint32_t size) {
    // First look for existing array types with the same element type.
    auto element_it = cfg.m_types.arrays.find(element);

    if (element_it != cfg.m_types.arrays.end()) {
        // Now, check if there is an existing array type of the same size.
        auto size_it = element_it->second.find(size);

        if (size_it != element_it->second.end()) {
            return size_it->second;
        } else {
            // No size match, so create a new type.
            ArrayType *type = new ArrayType(element, size);
            assert(type);

            element_it->second.emplace(size, type);
            return type;
        }
    } else {
        // No element match, so create a new mapping and a new type.
        ArrayType *type = new ArrayType(element, size);
        assert(type);

        cfg.m_types.arrays.emplace(type, std::unordered_map<uint32_t, ArrayType*>());
        cfg.m_types.arrays[type].emplace(size, type);
        return type;
    }
}

//>==---------------------------------------------------------------------------
//                          PointerType Implementation
//>==---------------------------------------------------------------------------

PointerType *PointerType::get(CFG &cfg, Type *pointee) {
    auto it = cfg.m_types.pointers.find(pointee);
    if (it != cfg.m_types.pointers.end())
        return it->second;

    PointerType *type = new PointerType(pointee);
    assert(type);

    cfg.m_types.pointers.emplace(pointee, type);
    return type;
}

//>==---------------------------------------------------------------------------
//                          FunctionType Implementation
//>==---------------------------------------------------------------------------

FunctionType *FunctionType::get(CFG &cfg, const Params &params, Type *result) {
    FunctionType *type = new FunctionType(params, result);
    assert(type);

    cfg.m_types.functions.push_back(type);
    return type;
}

std::string FunctionType::to_string() const {
    std::string str = "(";

    for (uint32_t i = 0, e = num_params(); i < e; ++i) {
        str += m_params[i]->to_string();
        if (i + 1 != e)
            str += ", ";
    }

    str += ")";

    if (has_result())
        str += " -> " + m_result->to_string();
    
    return str;
}

//>==---------------------------------------------------------------------------
//                          StructType Implementation
//>==---------------------------------------------------------------------------

StructType *StructType::get(CFG &cfg, const std::string &name) {
    auto it = cfg.m_types.structs.find(name);
    if (it != cfg.m_types.structs.end())
        return it->second;

    return nullptr;
}

StructType *StructType::create(CFG &cfg, const std::string &name,
                               const Fields &fields) {
    assert(!get(cfg, name) && "structure with name already exists!");

    StructType *type = new StructType(name, fields);
    assert(type);

    cfg.m_types.structs.emplace(name, type);
    return type;
}
