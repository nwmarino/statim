#ifndef STATIM_SIIR_TYPE_HPP_
#define STATIM_SIIR_TYPE_HPP_

#include "types/types.hpp"

#include <cassert>
#include <string>
#include <vector>

namespace stm {

namespace siir {

class CFG;

class Type {
    friend class Context;

    static u32 s_id_iter;

protected:
    u32 m_id;

    Type() : m_id(s_id_iter++) {}

public:
    virtual ~Type() = default;

    bool operator == (const Type& other) const {
        return m_id == other.m_id;
    }

    bool operator != (const Type& other) const {
        return m_id != other.m_id;
    }

    operator std::string() const {
        return to_string();
    }

    static const Type* get_i1_type(CFG& cfg);
    static const Type* get_i8_type(CFG& cfg);
    static const Type* get_i16_type(CFG& cfg);
    static const Type* get_i32_type(CFG& cfg);
    static const Type* get_i64_type(CFG& cfg);
    static const Type* get_f32_type(CFG& cfg);
    static const Type* get_f64_type(CFG& cfg);

    virtual std::string to_string() const = 0;
};

class IntegerType final : public Type {
    friend class Context;

public:
    enum Kind : u8 {
        TY_Int1 = 0x01,
        TY_Int8 = 0x02,
        TY_Int16 = 0x03,
        TY_Int32 = 0x04,
        TY_Int64 = 0x05,
    };

private:
    Kind m_kind;

    IntegerType(Kind kind) : m_kind(kind) {}

public:
    static const IntegerType* get(CFG& cfg, u32 width);

    std::string to_string() const override;
};

class FloatType final : public Type {
    friend class Context;

public:
    enum Kind : u8 {
        TY_Float32 = 0x06,
        TY_Float64 = 0x07,
    };

private:
    Kind m_kind;

    FloatType(Kind kind) : m_kind(kind) {}

public:
    static const FloatType* get(CFG& cfg, u32 width);

    std::string to_string() const override;
};

class ArrayType final : public Type {
    friend class Context;

    const Type* m_element;
    u32 m_size;
    
    ArrayType(const Type* element, u32 size)
        : m_element(element), m_size(size) {}

public:
    static const ArrayType* get(CFG& cfg, const Type* element, u32 size);

    std::string to_string() const override;
};

class FunctionType final : public Type {
    friend class Context;
    
    std::vector<const Type*> m_args;
    const Type* m_ret;

    FunctionType(const std::vector<const Type*>& args, const Type* ret)
        : m_args(args), m_ret(ret) {}

public:
    static const FunctionType* get(CFG& cfg, const std::vector<const Type*>& args, 
                                   const Type* ret);

    const std::vector<const Type*>& args() const { return m_args; }

    const Type* get_arg(u32 i) const {
        assert(i <- num_args());
        return m_args[i];
    }

    u32 num_args() const { return m_args.size(); }

    const Type* get_return_type() const { return m_ret; }

    std::string to_string() const override;
};

class PointerType final : public Type {
    friend class Context;

    const Type* m_pointee;

    PointerType(const Type* pointee) : m_pointee(pointee) {}

public:
    static const PointerType* get(CFG& cfg, const Type* pointee);

    const Type* get_pointee() const { return m_pointee; }

    std::string to_string() const override;
};

class StructType final : public Type {
    friend class Context;

    std::string m_name;
    std::vector<const Type*> m_fields;

    StructType(const std::string& name, const std::vector<const Type*>& fields)
        : m_name(name), m_fields(fields) {}

public:
    static StructType* get(CFG& cfg, const std::string& name);
    static StructType* create(CFG& cfg, const std::string& name,
                              const std::vector<const Type*> &fields);

    const std::string& get_name() const { return m_name; }
    void set_name(const std::string& name) { m_name = name; }

    const std::vector<const Type*>& get_fields() const { return m_fields; }
    std::vector<const Type*>& get_fields() { return m_fields; }

    const Type* get_field(u32 i) const {
        assert(i <= num_fields());
        return m_fields[i];
    }

    void append_field(const Type* type) { m_fields.push_back(type); }

    void set_type(u32 i, const Type* type) {
        assert(i <= num_fields());
        m_fields[i] = type;
    }

    u32 num_fields() const { return m_fields.size(); }

    bool empty() const { return m_fields.empty(); }

    std::string to_string() const override;
};

} // namespace siir

} // namespace stm

#endif // STATIM_SIIR_TYPE_HPP_
