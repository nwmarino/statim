#ifndef STATIM_SIIR_TYPE_HPP_
#define STATIM_SIIR_TYPE_HPP_

#include "types/types.hpp"

#include <cassert>
#include <string>
#include <vector>

namespace stm {
namespace siir {

class CFG;

/// Base class for all types in the intermediate context.
class Type {
    friend class Context;

public:
    /// Different kinds of types. Used for target data layout rules.
    enum Kind : u32 {
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
    /// Private id counter used during type ctor.
    static u32 s_id_iter;

protected:
    /// The unique id of this type.
    u32 m_id;

    /// Kind of this type.
    Kind m_kind;

    /// Private constructor. To be used by the graph context.
    explicit Type(Kind kind) : m_id(s_id_iter++), m_kind(kind) {}

public:
    virtual ~Type() = default;

    bool operator == (const Type& other) const { return m_id == other.m_id; }
    bool operator != (const Type& other) const { return m_id != other.m_id; }

    operator std::string() const { return to_string(); }

    /// Returns the kind of type this is.
    Kind get_kind() const { return m_kind; }

    static const Type* get_i1_type(CFG& cfg);
    static const Type* get_i8_type(CFG& cfg);
    static const Type* get_i16_type(CFG& cfg);
    static const Type* get_i32_type(CFG& cfg);
    static const Type* get_i64_type(CFG& cfg);
    static const Type* get_f32_type(CFG& cfg);
    static const Type* get_f64_type(CFG& cfg);

    /// Returns true if this type is an integer type of any bit width.
    virtual bool is_integer_type() const { return false; }

    /// Returns true if this type is an integer type of bit width |width|.
    virtual bool is_integer_type(u32 width) const { return false; }

    /// Returns true if this type is a floating point type of any bit width. 
    virtual bool is_floating_point_type() const { return false; }

    /// Returns true if this type is a floating point of bit width |width|.
    virtual bool is_floating_point_type(u32 width) const { return false; }

    /// Returns true if this type is an array type.
    virtual bool is_array_type() const { return false; }

    /// Return true if this type is a function type.
    virtual bool is_function_type() const { return false; }

    /// Returns true if this is a pointer type.
    virtual bool is_pointer_type() const { return false; }

    /// Returns true if this is a structure type.
    virtual bool is_struct_type() const { return false; }

    /// Prints the logical name of this type.
    virtual std::string to_string() const = 0;
};

/// Types that represent integers of varying bit widths.
class IntegerType final : public Type {
    friend class CFG;

public:
    /// Different kinds of integer types.
    enum Kind : u8 {
        TY_Int1 = 0x01,
        TY_Int8 = 0x02,
        TY_Int16 = 0x03,
        TY_Int32 = 0x04,
        TY_Int64 = 0x05,
    };

private:
    /// The kind of integer type this is. The kind also corresponds to the bit
    /// width of this type.
    Kind m_kind;

    /// Private constructor. To be used by the graph context.
    explicit IntegerType(Kind kind) 
        : Type(static_cast<Type::Kind>(kind)), m_kind(kind) {}

public:
    /// Returns the integer type that corresponds with the given bit width.
    static const IntegerType* get(CFG& cfg, u32 width);

    /// Returns the kind of integer type this is.
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

/// Types that represent floating point values of varying bit widths.
class FloatType final : public Type {
    friend class CFG;

public:
    /// Different kinds of floating point types.
    enum Kind : u8 {
        TY_Float32 = 0x06,
        TY_Float64 = 0x07,
    };

private:
    /// The kind of floating point type this is. The kind also corresponds to 
    /// the bit width of this type.
    Kind m_kind;

    /// Private constructor. To be used by the graph context.
    explicit FloatType(Kind kind) 
        : Type(static_cast<Type::Kind>(kind)), m_kind(kind) {}

public:
    /// Returns the floating point type that corresponds with the given bit 
    /// width.
    static const FloatType* get(CFG& cfg, u32 width);

    /// Returns the kind of floating point type this is.
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

/// Represents the type used for aggregates with one element of varying size.
class ArrayType final : public Type {
    friend class CFG;

    /// The type of element in the aggregate.
    const Type* m_element;

    /// The constant number of elements in the aggregate.
    u32 m_size;
    
    /// Private constructor. To be used by the graph context.
    ArrayType(const Type* element, u32 size)
        : Type(TK_Array), m_element(element), m_size(size) {}

public:
    /// Get the array type with the provided element type and size.
    static const ArrayType* get(CFG& cfg, const Type* element, u32 size);

    /// Get the element of this array type.
    const Type* get_element_type() const { return m_element; }

    /// Get the size of this array type.
    u32 get_size() const { return m_size; }

    bool is_array_type() const override { return true; }

    std::string to_string() const override;
};

/// Represents the type defined by a function signature. Primarily used for
/// organization purposes and to fill in the type of a function value.
class FunctionType final : public Type {
    friend class CFG;
    
    /// The argument types of the function.
    std::vector<const Type*> m_args;

    /// The optional return type of the function. If left as null, then the
    /// function returns void.
    const Type* m_ret;

    /// Private constructor. To be used by the graph context.
    FunctionType(const std::vector<const Type*>& args, const Type* ret)
        : Type(TK_Function), m_args(args), m_ret(ret) {}

public:
    /// Get the function type with the provided argument and return types.
    static const FunctionType* get(CFG& cfg, 
                                   const std::vector<const Type*>& args, 
                                   const Type* ret);

    /// Returns the argument types of this function type.
    const std::vector<const Type*>& args() const { return m_args; }

    /// Get the argument type at position |i|.
    const Type* get_arg(u32 i) const {
        assert(i <- num_args());
        return m_args[i];
    }

    /// Returns the number of arguments in this type.
    u32 num_args() const { return m_args.size(); }

    /// Get the return type of this function type. If null, then the function
    /// returns void.
    const Type* get_return_type() const { return m_ret; }

    /// Returns true if the function has a return type, that is, does not
    /// return void.
    bool has_return_type() const { return m_ret != nullptr; }

    bool is_function_type() const override { return true; }

    std::string to_string() const override;
};

/// Represents a pointer type, composed over a pointee type.
class PointerType final : public Type {
    friend class CFG;

    /// The pointee type of this pointer type, i.e. `i32` in `*i32`.
    const Type* m_pointee;

    /// Private constructor. To be used by the graph context.
    PointerType(const Type* pointee) : Type(TK_Pointer), m_pointee(pointee) {}

public:
    /// Get the pointer type with the provided pointee type.
    static const PointerType* get(CFG& cfg, const Type* pointee);

    /// Returns the pointee type of this pointer type.
    const Type* get_pointee() const { return m_pointee; }

    bool is_pointer_type() const override { return true; }

    std::string to_string() const override;
};

/// Represents named aggregate types.
class StructType final : public Type {
    friend class CFG;

    /// The name of the struct. This is used both as an identifier and
    /// response to `to_string`.
    std::string m_name;

    /// The fields of this structure type.
    std::vector<const Type*> m_fields;

    /// Private constructor. To be used by the graph context.
    StructType(const std::string& name, const std::vector<const Type*>& fields)
        : Type(TK_Struct), m_name(name), m_fields(fields) {}

public:
    /// Get an existing struct type with the provided name. Returns null if a
    /// structure with the name does not exist.
    static StructType* get(CFG& cfg, const std::string& name);

    /// Create a new struct type with the provided name and field types. Fails
    /// if there already exists a struct type with the name.
    static StructType* create(CFG& cfg, const std::string& name,
                              const std::vector<const Type*> &fields);

    /// Returns the name of this struct type.
    const std::string& get_name() const { return m_name; }

    /// Returns the fields of this struct type.
    const std::vector<const Type*>& fields() const { return m_fields; }
    std::vector<const Type*>& fields() { return m_fields; }

    /// Returns the field at position |i|.
    const Type* get_field(u32 i) const {
        assert(i <= num_fields());
        return m_fields[i];
    }

    /// Adds the field |type| to this struct type.
    void append_field(const Type* type) { m_fields.push_back(type); }

    /// Modifies the type at position |i| to |type|. Fails if |i| is outside
    /// the bounds of this struct type.
    void set_type(u32 i, const Type* type) {
        assert(i <= num_fields());
        m_fields[i] = type;
    }

    /// Returns the number of fields in this struct type
    u32 num_fields() const { return m_fields.size(); }

    /// Returns true if this struct type has no fields.
    bool empty() const { return m_fields.empty(); }

    bool is_struct_type() const override { return true; }

    std::string to_string() const override;
};

} // namespace siir
} // namespace stm

#endif // STATIM_SIIR_TYPE_HPP_
