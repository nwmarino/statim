#include "siir/cfg.hpp"
#include "siir/type.hpp"
#include "siir/constant.hpp"

using namespace stm;
using namespace stm::siir;

Constant* ConstantInt::get_true(CFG& cfg) {
    return cfg.m_int1_one;
}

Constant* ConstantInt::get_false(CFG& cfg) {
    return cfg.m_int1_zero;
}

Constant* ConstantInt::get_zero(CFG& cfg, const Type* type) {
    return get(cfg, type, 0);
}

Constant* ConstantInt::get_one(CFG& cfg, const Type* type) {
    return get(cfg, type, 1);
}

Constant* ConstantInt::get(CFG& cfg, const Type* type, i64 value) {
    assert(type->is_integer_type() && 
        "integer constant type must be an integer");

    switch (static_cast<const IntegerType*>(type)->get_kind()) {
        case IntegerType::TY_Int1:
            return value == 0 ? cfg.m_int1_zero : cfg.m_int1_one;

        case IntegerType::TY_Int8: {
            auto it = cfg.m_pool_int8.find(value);
            if (it != cfg.m_pool_int8.end())
                return it->second;

            ConstantInt* cint = new ConstantInt(value, type);
            cfg.m_pool_int8.emplace(value, cint);
            return cint;
        }
            
        case IntegerType::TY_Int16: {
            auto it = cfg.m_pool_int16.find(value);
            if (it != cfg.m_pool_int16.end())
                return it->second;

            ConstantInt* cint = new ConstantInt(value, type);
            cfg.m_pool_int16.emplace(value, cint);
            return cint;
        }
        
        case IntegerType::TY_Int32: {
            auto it = cfg.m_pool_int32.find(value);
            if (it != cfg.m_pool_int32.end())
                return it->second;

            ConstantInt* cint = new ConstantInt(value, type);
            cfg.m_pool_int32.emplace(value, cint);
            return cint;
        }
        
        case IntegerType::TY_Int64: {
            auto it = cfg.m_pool_int64.find(value);
            if (it != cfg.m_pool_int64.end())
                return it->second;

            ConstantInt* cint = new ConstantInt(value, type);
            cfg.m_pool_int64.emplace(value, cint);
            return cint;
        }
    }
}

Constant* ConstantFP::get_zero(CFG& cfg, const Type* type) {
    return get(cfg, type, 0);
}

Constant* ConstantFP::get_one(CFG& cfg, const Type* type) {
    return get(cfg, type, 1);
}

Constant* ConstantFP::get(CFG& cfg, const Type* type, f64 value) {
    assert(type->is_floating_point_type() && 
        "floating point constant type must be a float");

    switch (static_cast<const FloatType*>(type)->get_kind()) {
        case FloatType::TY_Float32: {
            auto it = cfg.m_pool_fp32.find(value);
            if (it != cfg.m_pool_fp32.end())
                return it->second;

            ConstantFP* cfp = new ConstantFP(value, type);
            cfg.m_pool_fp32.emplace(value, cfp);
            return cfp;
        }

        case FloatType::TY_Float64: {
            auto it = cfg.m_pool_fp64.find(value);
            if (it != cfg.m_pool_fp64.end())
                return it->second;

            ConstantFP* cfp = new ConstantFP(value, type);
            cfg.m_pool_fp64.emplace(value, cfp);
            return cfp;
        }
    }
}

Constant* ConstantNull::get(CFG& cfg, const Type* type) {
    assert(type);

    auto it = cfg.m_pool_null.find(type);
    if (it != cfg.m_pool_null.end())
        return it->second;

    ConstantNull* null = new ConstantNull(type);
    cfg.m_pool_null.emplace(type, null);
    return null;
}

Constant* BlockAddress::get(CFG& cfg, BasicBlock* blk) {
    assert(blk);

    auto it = cfg.m_pool_baddr.find(blk);
    if (it != cfg.m_pool_baddr.end())
        return it->second;

    BlockAddress* null = new BlockAddress(blk);
    cfg.m_pool_baddr.emplace(blk, null);
    return null;
}

ConstantString* ConstantString::get(CFG& cfg, const std::string& string) {
    auto it = cfg.m_pool_str.find(string);
    if (it != cfg.m_pool_str.end())
        return it->second;

    ConstantString* str = new ConstantString(
        string, 
        PointerType::get(cfg, Type::get_i8_type(cfg)));
        //ArrayType::get(cfg, Type::get_i8_type(cfg), string.size() + 1));

    cfg.m_pool_str.emplace(string, str);
    return str;
}
