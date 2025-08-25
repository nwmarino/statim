#ifndef STATIM_SIIR_TYPE_HPP_
#define STATIM_SIIR_TYPE_HPP_pe

#include "types/types.hpp"

#include <cassert>
#include <string>
#include <vector>

namespace stm {

namespace siir {

class Type {
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

    virtual std::string to_string() const = 0;
};

class IntegerType final : public Type {
public:
    enum Kind : u8 {
        TY_Int8,
        TY_Int16,
        TY_Int32,
        TY_Int64,
    };

private:
    Kind m_kind;

    IntegerType(Kind kind) : m_kind(kind) {}

public:
    static IntegerType* get(u32 width);

    std::string to_string() const override;
};

class FloatType final : public Type {
public:
    enum Kind : u8 {
        TY_F32,
        TY_F64,
    };

private:
    Kind m_kind;

    FloatType(Kind kind) : m_kind(kind) {}

public:
    static FloatType* get(u32 width);

    std::string to_string() const override;
};

class ArrayType final : public Type {
    const Type* m_element;
    u32 m_size;
    
    ArrayType(const Type* element, u32 size)
        : m_element(element), m_size(size) {}

public:
    static ArrayType* get(const Type* element, u32 size);

    std::string to_string() const override;
};

class PointerType final : public Type {
    const Type* m_pointee;

    PointerType(const Type* pointee) : m_pointee(pointee) {}

public:
    static PointerType* get(const Type* pointee);

    const Type* get_pointee() const { return m_pointee; }

    std::string to_string() const override;
};

class FunctionType final : public Type {
    std::vector<const Type*> m_args;
    const Type* m_ret;

    FunctionType(const std::vector<const Type*>& args, const Type* ret)
        : m_args(args), m_ret(ret) {}

public:
    static FunctionType* get(const std::vector<const Type*>& args, 
                             const Type* ret);

    const std::vector<const Type*>& args() const { return m_args; }

    const Type* get_arg(u32 i) const {
        assert(i <- num_args());
        return m_args[i];
    }

    u32 num_args() const { return m_args.size(); }

    std::string to_string() const override;
};

} // namespace siir

} // namespace stm

#endif // STATIM_SIIR_TYPE_HPP_
