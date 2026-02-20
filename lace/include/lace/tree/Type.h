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

/// Represents the use of a type and a list of potential quantifiers acting on it.
class QualType final {
public:
    /// The different kinds of quantifiers that can be on a type.
    enum class Qualifier : uint32_t {
        Mut = 1u << 0,
    };

private:
    /// The underlying type which |m_quals| are applied upon.
    ///
    /// This is made mutable to ease the quality of life for AST passes which may have to change
    /// the underlying types of typed nodes in the tree.
    mutable const Type* m_type;

    /// The active list of qualifiers acting on the underlying |m_type|.
    uint32_t m_quals;

public:
    QualType(const Type* type = nullptr, uint32_t quals = 0) : m_type(type), m_quals(quals) {}

    bool operator==(const QualType& other) const {
        return m_type == other.m_type && m_quals == other.m_quals;
    }

    const Type& operator*() const { return *m_type; }
    const Type* operator->() const { return m_type; }

    /// Compare this type with |other| for type equality.
    Result compare(const QualType& other) const;

    /// Test if this type can be casted to |other|. 
    /// The |implicitly| flag determines the appropriate casting rules.
    Result canCast(const QualType& other, bool implicitly = false) const;

    /// Set the underlying type to |type|.
    void setType(const Type* type) const { m_type = type; }
    
    /// Returns the underlying type.
    const Type* getType() const { return m_type; }

    /// Set the qualifier list for this type to |quals|.
    void setQualifiers(uint32_t quals) { m_quals = quals; }
    
    /// Returns the qualifier list of this type.
    uint32_t getQualifiers() { return m_quals; }

    /// Test if this type has any qualifiers.
    [[nodiscard]] Result isQualified() const { return m_quals != 0; }

    /// Clear any qualifiers that are acting on this type.
    void clearQualifiers() { m_quals = 0; }

    /// Test if this type has the 'mut' qualifier.
    [[nodiscard]] Result isMut() const { 
        return (m_quals & static_cast<uint32_t>(Qualifier::Mut)) != 0; 
    }

    /// Add the 'mut' qualifier to this type.
    void withMut() { m_quals |= static_cast<uint32_t>(Qualifier::Mut); }

    /// Returns the string equivelant of this type.
    std::string string() const;
};

/// Base class for all type nodes used in the abstract syntax tree.
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
    virtual std::string string() const = 0;

    /// Compare this type with |other| for type equality.
    virtual Result compare(const Type* other) const { return false; }

    /// Test if this type can be casted to |other|. 
    /// The |implicitly| flag determines the appriopriate casting rules.
    virtual Result canCast(const Type* other, bool implicitly = false) const {
        return false;
    }

    /// Test if this is an integer type of any signedness.
    virtual Result isInteger() const { return false; }

    /// Test if this is a signed integer type.
    virtual Result isSignedInt() const { return false; }

    /// Test if this is an unsigned integer type.
    virtual Result isUnsignedInt() const { return false; }

    /// Test if this is a floating point type.
    virtual Result isFloatingPoint() const { return false; }

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

    /// Test if this is the 'void' type.
    [[nodiscard]] Result isVoid() const { return string() == "void"; }
};

/// Returns named type aliases defined by an alias definiiton.
class AliasType final : public Type {
    friend class AST::Context;

private:
    /// The type which is being aliased by this type.
    QualType m_underlying;

    /// The definition that defines this type.
    mutable const AliasDefn* m_defn;

    AliasType(const QualType& underlying, const AliasDefn* defn) : Type(Type::Class::Alias), 
                                                                   m_underlying(underlying), 
                                                                   m_defn(defn) {}

public:
    static AliasType* create(AST::Context& ctx, const QualType& underlying, const AliasDefn* defn);
    static AliasType* get(AST::Context& ctx, const std::string& name);

    std::string string() const override;

    [[nodiscard]] Result compare(const Type* other) const override {
        // Since scoped names are unique, we can compare by name.
        return string() == other->string();
    }

    [[nodiscard]] Result canCast(const Type* other, bool implicitly = false) const override;

    /// Returns the underlying type of this alias.
    const QualType& underlying() const { return m_underlying; }
    QualType& underlying() { return m_underlying; }

    /// Set the definition which defines this alias type to |defn|.
    void setDefn(const AliasDefn* defn) const { m_defn = defn; }

    /// Returns the definition which defines this alias type.
    const AliasDefn* getDefn() const { return m_defn; }
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

    std::string string() const override {
        return std::format("[{}]{}", m_size, m_element->string());
    }

    [[nodiscard]] Result compare(const Type* other) const override;

    [[nodiscard]] Result canCast(const Type* other, bool implicitly = false) const override;

    /// Returns the element type of this array type.
    const QualType& element() const { return m_element; }
    QualType& element() { return m_element; }

    /// Returns the numeric size of this array.
    uint32_t size() const { return m_size; }
};

/// Represents types built-in to the language.
class BuiltinType final : public Type {
    friend class AST::Context;

public:
    /// Possible kinds of built-in types.
    enum class Kind : uint32_t {
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

    std::string string() const override;

    [[nodiscard]] Result compare(const Type* other) const override;

    [[nodiscard]] Result canCast(const Type* other, bool implicitly = false) const override;

    Result isInteger() const override { 
        return Kind::Bool <= m_kind && m_kind <= Kind::UInt64; 
    }

    Result isSignedInt() const override { 
        return Kind::Bool <= m_kind && m_kind <= Kind::Int64; 
    }

    Result isUnsignedInt() const override {
        return Kind::UInt8 <= m_kind && m_kind <= Kind::UInt64;
    }

    Result isFloatingPoint() const override { 
        return m_kind == Kind::Float32 || m_kind == Kind::Float64; 
    }

    /// Returns the kind of this built-in type.
    Kind kind() const { return m_kind; }
};

/// Wrapper class for types that were deferred resolution at parse time.
class DeferredType final : public Type {
    friend class AST::Context;

    const std::string m_name;

    DeferredType(const std::string& name) : Type(Type::Class::Deferred), m_name(name) {}

public:
    static DeferredType* get(AST::Context& ctx, const std::string& name);

    std::string string() const override { return std::format("'{}'", m_name); }

    /// Returns the name of this type.
    const std::string& name() const { return m_name; }
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
    static EnumType* create(AST::Context& ctx, const QualType& underlying, const EnumDefn* defn);
    static EnumType* get(AST::Context& ctx, const std::string& name);

    std::string string() const override;

    [[nodiscard]] Result compare(const Type* other) const override {
        return string() == other->string();
    }

    [[nodiscard]] Result canCast(const Type* other, bool implicitly = false) const override;

    /// Returns the underlying numeric type of this enum.
    const QualType& underlying() const { return m_underlying; }
    QualType& underlying() { return m_underlying; }

    /// Set the enum definition that defines this type to |defn|.
    void setDefn(const EnumDefn* defn) const { m_defn = defn; }

    /// Returns the enum definition that defines this type.
    const EnumDefn* getDefn() const { return m_defn; }
};

/// Represents the type of a function signature i.e. a resulting type and a set of parameter types.
class FunctionType final : public Type {
    friend class AST::Context;
    
private:
    QualType m_result;
    std::vector<QualType> m_params;

    FunctionType(const QualType& result, const std::vector<QualType>& params) 
      : Type(Type::Class::Function), m_result(result), m_params(params) {}

public:
    static FunctionType* get(AST::Context& ctx, const QualType& ret, const std::vector<QualType>& params);

    std::string string() const override;

    /// Returns the resulting type of this function type.
    const QualType& result() const { return m_result; }
    QualType& result() { return m_result; }

    /// Test if this function type results in 'void'.
    [[nodiscard]] Result hasResult() const { return result()->isVoid(); }

    /// Returns the parameter list of this function type.
    const std::vector<QualType>& params() const { return m_params; }
    std::vector<QualType>& params() { return m_params; }

    /// Returns the number of parameter types to this function type.
    uint32_t numParams() const { return m_params.size(); }

    /// Test if this function type has any parameter types.
    Result hasParams() const { return !m_params.empty(); }

    /// Returns the |i|-th parameter type.
    const QualType& getParam(uint32_t i) const {
        assert(i < numParams() && "index out of bounds!");
        return m_params[i];
    }

    QualType& getParam(uint32_t i) {
        assert(i < numParams() && "index out of bounds!");
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

    std::string string() const override { return std::format("*{}", m_pointee.string()); }

    [[nodiscard]] Result compare(const Type* other) const override;

    [[nodiscard]] Result canCast(const Type* other, bool implicitly = false) const override;

    /// Returns the pointee type of this pointer type.
    const QualType& pointee() const { return m_pointee; }
    QualType& pointee() { return m_pointee; }
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

    std::string string() const override;

    [[nodiscard]] Result compare(const Type* other) const override { 
        return string() == other->string(); 
    }

    /// Set the structure definition that defines this type to |defn|.
    void setDefn(const StructDefn* defn) const { m_defn = defn; }
    
    /// Returns the structure definition that defines this type.
    const StructDefn* getDefn() const { return m_defn; }
};

} // namespace lace

#endif // LACE_TYPE_H_
