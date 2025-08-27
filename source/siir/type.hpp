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

public:
    enum Kind : u8 {
        TK_Int1 = 0x01,
        TK_Int8 = 0x02,
        TK_Int16 = 0x03,
        TK_Int32 = 0x04,
        TK_Int64 = 0x05,
        TK_Float32 = 0x06,
        TK_Float64 = 0x07,
        TK_Array = 0x08,
        TK_Function = 0x09,
        TK_Pointer = 0x10,
        TK_Struct = 0x11,
    };

private:
    static u32 s_id_iter;

protected:
    u32 m_id;
    Kind m_kind;

    Type(Kind kind) : m_id(s_id_iter++), m_kind(kind) {}

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

    Kind get_kind() const { return m_kind; }

    static const Type* get_i1_type(CFG& cfg);
    static const Type* get_i8_type(CFG& cfg);
    static const Type* get_i16_type(CFG& cfg);
    static const Type* get_i32_type(CFG& cfg);
    static const Type* get_i64_type(CFG& cfg);
    static const Type* get_f32_type(CFG& cfg);
    static const Type* get_f64_type(CFG& cfg);

    virtual bool is_integer_type() const { return false; }
    virtual bool is_integer_type(u32 width) const { return false; }

    virtual bool is_floating_point_type() const { return false; }
    virtual bool is_floating_point_type(u32 width) const { return false; }

    virtual bool is_array_type() const { return false; }

    virtual bool is_function_type() const { return false; }

    virtual bool is_pointer_type() const { return false; }

    virtual bool is_struct_type() const { return false; }

    virtual std::string to_string() const = 0;
};

class IntegerType final : public Type {
    friend class CFG;

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

    IntegerType(Kind kind) 
        : Type(static_cast<Type::Kind>(kind)), m_kind(kind) {}

public:
    static const IntegerType* get(CFG& cfg, u32 width);

    Kind get_kind() const { return m_kind; }

    bool is_integer_type() const override { return true; }

    bool is_integer_type(u32 width) const override {
        switch (width) {
        case 1:
            return m_kind == TY_Int1;
        case 8:
            return m_kind == TY_Int8;
        case 16:
            return m_kind == TY_Int16;
        case 32:
            return m_kind == TY_Int32;
        case 64:
            return m_kind == TY_Int64;
        }

        return false;
    }

    std::string to_string() const override;
};

class FloatType final : public Type {
    friend class CFG;

public:
    enum Kind : u8 {
        TY_Float32 = 0x06,
        TY_Float64 = 0x07,
    };

private:
    Kind m_kind;

    FloatType(Kind kind) : Type(static_cast<Type::Kind>(kind)), m_kind(kind) {}

public:
    static const FloatType* get(CFG& cfg, u32 width);

    Kind get_kind() const { return m_kind; }

    bool is_floating_point_type() const override { return true; }

    bool is_floating_point_type(u32 width) const override { 
        switch (width) {
        case 32:
            return m_kind == TY_Float32;
        case 64:
            return m_kind == TY_Float64;
        }

        return false;
    }

    std::string to_string() const override;
};

class ArrayType final : public Type {
    friend class CFG;

    const Type* m_element;
    u32 m_size;
    
    ArrayType(const Type* element, u32 size)
        : Type(TK_Array), m_element(element), m_size(size) {}

public:
    static const ArrayType* get(CFG& cfg, const Type* element, u32 size);

    const Type* get_element_type() const { return m_element; }
    u32 get_size() const { return m_size; }

    bool is_array_type() const override { return true; }

    std::string to_string() const override;
};

class FunctionType final : public Type {
    friend class CFG;
    
    std::vector<const Type*> m_args;
    const Type* m_ret;

    FunctionType(const std::vector<const Type*>& args, const Type* ret)
        : Type(TK_Function), m_args(args), m_ret(ret) {}

public:
    static const FunctionType* get(CFG& cfg, 
                                   const std::vector<const Type*>& args, 
                                   const Type* ret);

    const std::vector<const Type*>& args() const { return m_args; }

    const Type* get_arg(u32 i) const {
        assert(i <- num_args());
        return m_args[i];
    }

    u32 num_args() const { return m_args.size(); }

    const Type* get_return_type() const { return m_ret; }

    bool has_return_type() const { return m_ret != nullptr; }

    bool is_function_type() const override { return true; }

    std::string to_string() const override;
};

class PointerType final : public Type {
    friend class CFG;

    const Type* m_pointee;

    PointerType(const Type* pointee) : Type(TK_Pointer), m_pointee(pointee) {}

public:
    static const PointerType* get(CFG& cfg, const Type* pointee);

    const Type* get_pointee() const { return m_pointee; }

    bool is_pointer_type() const override { return true; }

    std::string to_string() const override;
};

class StructType final : public Type {
    friend class CFG;

    std::string m_name;
    std::vector<const Type*> m_fields;

    StructType(const std::string& name, const std::vector<const Type*>& fields)
        : Type(TK_Struct), m_name(name), m_fields(fields) {}

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

    bool is_struct_type() const override { return true; }

    std::string to_string() const override;

    void print(std::ostream& os) const;
};

} // namespace siir

} // namespace stm

#endif // STATIM_SIIR_TYPE_HPP_
