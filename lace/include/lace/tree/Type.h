//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LACE_TYPE_H_
#define LACE_TYPE_H_

//
//  This header file contains definitions that make up the representation of types in the language 
//  type system.
//

#include "lace/core/Common.h"
#include "lace/tree/AST.h"

#include <cassert>
#include <cstdint>
#include <format>
#include <string>
#include <vector>

namespace lace {

class AliasDefn;
class Context;
class EnumDefn;
class StructDefn;
class Type;

/// Represents the use of a type and possible quantifiers over it.
class QualType final {
public:
    /// The different kinds of quantifiers that can be on a type.
    enum Qualifier : uint32_t {
        Mut = 1u << 0,
    };

private:
    mutable const Type* m_type;
    uint32_t m_quals;

public:
    QualType(const Type* type = nullptr, uint32_t quals = 0)
      : m_type(type), m_quals(quals) {}

    bool operator==(const QualType& other) const {
        return m_type == other.m_type && m_quals == other.m_quals;
    }

    const Type& operator*() const { return *m_type; }
    const Type* operator->() const { return m_type; }

    /// Compare this type with |other| for type equality.
    bool compare(const QualType& other) const;

    /// Test if this type can be casted to |other|. The |implicitly| flag
    /// determines if casting should follow implicit or explicit casting rules.
    bool can_cast(const QualType& other, bool implicitly = false) const;

    void set_type(const Type* type) const { m_type = type; }
    const Type* get_type() const { return m_type; }

    void set_qualifiers(uint32_t quals) { m_quals = quals; }
    const uint32_t& get_qualifiers() const { return m_quals; }
    uint32_t& get_qualifiers() { return m_quals; }

    /// Test if this type use has any qualifiers.
    bool is_qualified() const { return m_quals != 0; }

    /// Clear any qualifiers on this type.
    void clear_qualifiers() { m_quals = 0; }

    /// Test if this type has the `mut` qualifier.
    bool is_mut() const { return (m_quals & Mut) != 0; }

    /// Qualify this type with `mut`.
    void with_mut() { m_quals |= Mut; }

    /// Returns the string equivelant of this type.
    std::string to_string() const;
};

/// Base class for all type nodes used in the abstract syntax tree (AST).
class Type {
public:
    /// The different type classes.
    enum class Class : uint32_t {
        Alias,
        Array,
        Builtin,
        Deferred,
        Enum,
        Function,
        Pointer,
        Struct,
    };

protected:
    /// The class of this type.
    const Class m_class;

    Type(Class cls) : m_class(cls) {}

public:
    virtual ~Type() = default;

    /// Returns the string equivelant of this type.
    virtual std::string to_string() const = 0;

    /// Compare this type with |other| for type equality.
    virtual bool compare(const Type* other) const { return false; }

    /// Returns true if this type can be casted to |other|. The |implicitly|
    /// flag determines if the cast follows implicit or explicit casting rules.
    virtual bool can_cast(const Type* other, bool implicitly = false) const {
        return false;
    }

    /// Test if this is an integer type of any signedness.
    virtual bool is_integer() const { return false; }

    /// Test if this is a signed integer type.
    virtual bool is_signed_integer() const { return false; }

    /// Test if this is an unsigned integer type.
    virtual bool is_unsigned_integer() const { return false; }

    /// Test if this is a floating point type.
    virtual bool is_floating_point() const { return false; }

    /// Returns the class of this type.
    Class getClass() const { return m_class; }

    /// Test if this type is of the given |cls|.
    Result isClass(Class cls) const { return m_class == cls; }

    /// Test if this is an array type.
    Result isArray() const { return m_class == Class::Array; }

    /// Test if this is a pointer type.
    Result isPointer() const { return m_class == Class::Pointer; }

    /// Test if this is a structure type.
    Result isStruct() const { return m_class == Class::Struct; }

    /// Test if this is the `void` type.
    bool is_void() const { return to_string() == "void"; }
};

/// Returns named type aliases defined by an alias definiiton.
class AliasType final : public Type {
    friend class AST::Context;

private:
    QualType m_underlying;

    /// The definition that defines this type.
    mutable const AliasDefn* m_defn;

    AliasType(const QualType& underlying, const AliasDefn* defn) : Type(Type::Class::Alias), 
                                                                   m_underlying(underlying), 
                                                                   m_defn(defn) {}

public:
    static AliasType* create(AST::Context& ctx, const QualType& underlying,
                             const AliasDefn* defn);
    static AliasType* get(AST::Context& ctx, const std::string& name);

    std::string to_string() const override;

    bool compare(const Type* other) const override { 
        return to_string() == other->to_string();
    }

    bool can_cast(const Type* other, bool implicitly = false) const override;

    const QualType& get_underlying() const { return m_underlying; }
    QualType& get_underlying() { return m_underlying; }

    void set_defn(const AliasDefn* defn) const { m_defn = defn; }
    const AliasDefn* get_defn() const { return m_defn; }
};

/// Represents statically sized array types.
class ArrayType final : public Type {
    friend class AST::Context;

    QualType m_element;
    const uint32_t m_size;

    ArrayType(const QualType& element, uint32_t size) : Type(Type::Class::Array), 
                                                        m_element(element), m_size(size) {}

public:
    static ArrayType* get(AST::Context& ctx, const QualType& element, uint32_t size);

    std::string to_string() const override {
        return std::format("[{}]{}", m_size, m_element->to_string());
    }

    bool compare(const Type* other) const override;

    bool can_cast(const Type* other, bool implicitly = false) const override;

    const QualType& get_element_type() const { return m_element; }
    QualType& get_element_type() { return m_element; }

    uint32_t get_size() const { return m_size; }
};

/// Represents types built-in to the language.
class BuiltinType final : public Type {
    friend class AST::Context;

public:
    /// Possible kinds of built-in types.
    enum Kind : uint32_t {
        Void,
        Bool,
        Char,
        Int8,
        Int16,
        Int32,
        Int64,
        UInt8,
        UInt16,
        UInt32,
        UInt64,
        Float32,
        Float64,
    };

private:
    // The kind of built-in type this is.
    const Kind m_kind;

    BuiltinType(Kind kind) : Type(Type::Class::Builtin), m_kind(kind) {}

public:
    static BuiltinType* get(AST::Context& ctx, Kind kind);

    std::string to_string() const override;

    bool compare(const Type* other) const override;

    bool can_cast(const Type* other, bool implicitly = false) const override;

    bool is_integer() const override { 
        return Bool <= m_kind && m_kind <= UInt64; 
    }

    bool is_signed_integer() const override { 
        return Bool <= m_kind && m_kind <= Int64; 
    }

    bool is_unsigned_integer() const override {
        return UInt8 <= m_kind && m_kind <= UInt64;
    }

    bool is_floating_point() const override { 
        return m_kind == Float32 || m_kind == Float64; 
    }

    Kind get_kind() const { return m_kind; }
};

/// Wrapper class for types that were deferred resolution at parse time.
class DeferredType final : public Type {
    friend class AST::Context;

    const std::string m_name;

    DeferredType(const std::string& name) : Type(Type::Class::Deferred), m_name(name) {}

public:
    static DeferredType* get(AST::Context& ctx, const std::string& name);

    std::string to_string() const override { return "'" + m_name + "'"; }

    const std::string& get_name() const { return m_name; }
};

/// Represents named types defined by an enum definition.
class EnumType final : public Type {
    friend class AST::Context;

    QualType m_underlying;

    /// The definition that defines this type.
    mutable const EnumDefn* m_defn;

    EnumType(const QualType& underlying, const EnumDefn* defn) : Type(Type::Class::Enum), 
                                                                 m_underlying(underlying), 
                                                                 m_defn(defn) {}

public:
    static EnumType* create(AST::Context& ctx, const QualType& underlying, 
                            const EnumDefn* defn);
    static EnumType* get(AST::Context& ctx, const std::string& name);

    std::string to_string() const override;

    bool compare(const Type* other) const override {
        return to_string() == other->to_string();
    }

    bool can_cast(const Type* other, bool implicitly = false) const override;

    const QualType& get_underlying() const { return m_underlying; }
    QualType& get_underlying() { return m_underlying; }

    void set_defn(const EnumDefn* defn) const { m_defn = defn; }
    const EnumDefn* get_defn() const { return m_defn; }
};

/// Represents the type of a function signature i.e. a return type and a set of 
/// parameter types.
class FunctionType final : public Type {
    friend class AST::Context;
    
public:
    using Params = std::vector<QualType>;
    
private:
    QualType m_ret;
    Params m_params;

    FunctionType(const QualType& ret, const Params& params) : Type(Type::Class::Function), 
                                                              m_ret(ret), m_params(params) {}

public:
    static FunctionType* get(AST::Context& ctx, const QualType& ret, 
                             const Params& params);

    std::string to_string() const override;

    const QualType& get_return_type() const { return m_ret; }
    QualType& get_return_type() { return m_ret; }

    /// Test if this function returns the `void` type.
    bool is_void_return() const { return m_ret->is_void(); }

    const Params& get_params() const { return m_params; }
    Params& get_params() { return m_params; }

    /// Returns the number of parameter types in this function signature type.
    uint32_t num_params() const { return m_params.size(); }

    /// Test if this function signature type has any parameter types.
    bool has_params() const { return !m_params.empty(); }

    /// Returns the |i|-th parameter type.
    const QualType& get_param(uint32_t i) const {
        assert(i < num_params() && "index out of bounds!");
        return m_params[i];
    }

    QualType& get_param(uint32_t i) {
        assert(i < num_params() && "index out of bounds!");
        return m_params[i];
    }
};

/// Represents composite pointer types.
class PointerType final : public Type {
    friend class AST::Context;

    QualType m_pointee;

    PointerType(const QualType& pointee) : Type(Type::Class::Pointer), m_pointee(pointee) {}

public:
    static PointerType* get(AST::Context& ctx, const QualType& pointee);

    std::string to_string() const override { 
        return '*' + m_pointee.to_string(); 
    }

    bool compare(const Type* other) const override;

    bool can_cast(const Type* other, bool implicitly = false) const override;

    const QualType& get_pointee() const { return m_pointee; }
    QualType& get_pointee() { return m_pointee; }
};

/// Represents named types defined by a struct definition.
class StructType final : public Type {
    friend class AST::Context;

    /// The definition that defines this type.
    mutable const StructDefn* m_defn;

    StructType(const StructDefn* defn) : Type(Type::Class::Struct), m_defn(defn) {}

public:
    static StructType* create(AST::Context& ctx, const StructDefn* defn);
    static StructType* get(AST::Context& ctx, const std::string& name);    

    std::string to_string() const override;

    bool compare(const Type* other) const override { 
        return to_string() == other->to_string(); 
    }

    void set_defn(const StructDefn* defn) const { m_defn = defn; }
    const StructDefn* get_defn() const { return m_defn; }
};

} // namespace lace

#endif // LACE_TYPE_H_
