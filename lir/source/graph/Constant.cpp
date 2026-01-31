//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lir/graph/CFG.hpp"
#include "lir/graph/Constant.hpp"
#include "lir/graph/Type.hpp"
#include "lir/graph/Value.hpp"

#include <cstdint>
#include <format>

using namespace lir;

//>==---------------------------------------------------------------------------
//                          Integer Implementation
//>==---------------------------------------------------------------------------

Integer *Integer::get_true(CFG &cfg) {
    return get(cfg, Type::get_i8(cfg), 1);
}

Integer *Integer::get_false(CFG &cfg) {
    return get(cfg, Type::get_i8(cfg), 0);
}

Integer *Integer::get_zero(CFG &cfg, Type *type) {
    return get(cfg, type, 0);
}

Integer *Integer::get_one(CFG &cfg, Type *type) {
    return get(cfg, type, 1);
}

Integer *Integer::get(CFG &cfg, Type *type, int64_t value) {
    assert(type->is_integer_type() && "type must be an integer!");

    switch (static_cast<const IntegerType*>(type)->get_width()) {
        case 8: {
            auto it = cfg.m_constants.bytes.find(value);
            if (it != cfg.m_constants.bytes.end())
                return it->second;

            Integer *integer = new Integer(value, type);
            assert(integer);

            cfg.m_constants.bytes.emplace(value, integer);
            return integer;
        }
            
        case 16: {
            auto it = cfg.m_constants.shorts.find(value);
            if (it != cfg.m_constants.shorts.end())
                return it->second;

            Integer *integer = new Integer(value, type);
            assert(integer);

            cfg.m_constants.shorts.emplace(value, integer);
            return integer;
        }
        
        case 32: {
            auto it = cfg.m_constants.ints.find(value);
            if (it != cfg.m_constants.ints.end())
                return it->second;

            Integer *integer = new Integer(value, type);
            assert(integer);

            cfg.m_constants.ints.emplace(value, integer);
            return integer;
        }
        
        case 64: {
            auto it = cfg.m_constants.longs.find(value);
            if (it != cfg.m_constants.longs.end())
                return it->second;

            Integer *integer = new Integer(value, type);
            assert(integer);

            cfg.m_constants.longs.emplace(value, integer);
            return integer;
        }

        default:
            assert(false && "invalid integer bit width!");
    }
}

void Integer::print(std::ostream &os, PrintPolicy policy) const {
    assert(policy != PrintPolicy::Def && "integer cannot be defined!");

    if (policy == PrintPolicy::Use)
        os << std::format("{}: {}", m_value, m_type->to_string());
}

//>==---------------------------------------------------------------------------
//                          Float Implementation
//>==---------------------------------------------------------------------------

Float *Float::get_zero(CFG &cfg, Type *type) {
    return get(cfg, type, 0);
}

Float *Float::get_one(CFG &cfg, Type *type) {
    return get(cfg, type, 1);
}

Float *Float::get(CFG &cfg, Type *type, double value) {
    assert(type->is_float_type() && "type must be a float!");

    switch (static_cast<const FloatType*>(type)->get_width()) {
        case 32: {
            auto it = cfg.m_constants.floats.find(value);
            if (it != cfg.m_constants.floats.end())
                return it->second;

            Float *fp = new Float(value, type);
            assert(fp);

            cfg.m_constants.floats.emplace(value, fp);
            return fp;
        }

        case 64: {
            auto it = cfg.m_constants.doubles.find(value);
            if (it != cfg.m_constants.doubles.end())
                return it->second;

            Float *fp = new Float(value, type);
            assert(fp);

            cfg.m_constants.doubles.emplace(value, fp);
            return fp;
        }

        default:
            assert(false && "invalid floating point bit width!");
    }
}

void Float::print(std::ostream &os, PrintPolicy policy) const {
    assert(policy != PrintPolicy::Def && "float cannot be defined!");

    if (policy == PrintPolicy::Use)
        os << std::format("{:.5f}: {}", m_value, m_type->to_string());
}

//>==---------------------------------------------------------------------------
//                          Null Implementatiopn
//>==---------------------------------------------------------------------------

Null *Null::get(CFG &cfg, Type *type) {
    assert(type && "null must have a type!");

    auto it = cfg.m_constants.nulls.find(type);
    if (it != cfg.m_constants.nulls.end())
        return it->second;

    Null *null = new Null(type);
    assert(null);

    cfg.m_constants.nulls.emplace(type, null);
    return null;
}

void Null::print(std::ostream &os, PrintPolicy policy) const {
    assert(policy != PrintPolicy::Def && "integer cannot be defined!");

    if (policy == PrintPolicy::Use)
        os << std::format("null: {}", m_type->to_string());
}

//>==---------------------------------------------------------------------------
//                          String Implementatiopn
//>==---------------------------------------------------------------------------

String *String::get(CFG &cfg, const std::string &string) {
    auto it = cfg.m_constants.strings.find(string);
    if (it != cfg.m_constants.strings.end())
        return it->second;

    String *str = new String(PointerType::get(cfg, Type::get_i8(cfg)), string);
    assert(str);

    cfg.m_constants.strings.emplace(string, str);
    return str;
}

void String::print(std::ostream &os, PrintPolicy policy) const {
    assert(policy != PrintPolicy::Def && "integer cannot be defined!");

    if (policy == PrintPolicy::Use) {
        os << '"';

        for (uint32_t i = 0, e = m_value.size(); i < e; ++i) {
            switch (m_value[i]) {
                case '\\':
                    os << "\\\\";
                    break;
                case '\'':
                    os << "\\'";
                    break;
                case '\"':
                    os << "\\\"";
                    break;
                case '\n':
                    os << "\\n";
                    break;
                case '\t':
                    os << "\\t";
                    break;
                case '\r':
                    os << "\\r";
                    break;
                case '\b':
                    os << "\\b";
                    break;
                case '\0':
                    os << "\\0";
                    break;
                default:
                    os << m_value[i];
                    break;
            }
        }
        
        os << '"';
    }
}

//>==---------------------------------------------------------------------------
//                          Aggregate Implementatiopn
//>==---------------------------------------------------------------------------

Aggregate *Aggregate::get(CFG &cfg, Type *type, const Values &values) {
    std::vector<Value*> vs(values.begin(), values.end());
    
    Aggregate *agg = new Aggregate(type, vs);
    assert(agg);

    cfg.m_constants.aggregates.push_back(agg);
    return agg;
}

void Aggregate::print(std::ostream &os, PrintPolicy policy) const {
    assert(policy != PrintPolicy::Def && "integer cannot be defined!");

    if (policy == PrintPolicy::Use) {
        os << "{ ";
        for (uint32_t i = 0, e = num_operands(); i < e; ++i) {
            get_value(i)->print(os, PrintPolicy::Use);
            if (i + 1 != e)
                os << ", ";
        }

        os << std::format(" }}: {}", m_type->to_string());
    }
}
