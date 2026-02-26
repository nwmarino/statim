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

#include <cassert>
#include <cstdint>
#include <format>
#include <string>
#include <vector>

namespace lace {

class Rib;

class AliasDefn;
class Context;
class EnumDefn;
class StructDefn;
class Type;

/// Base class for all types used in the syntax tree.
class Type {
protected:
    Type() = default;

public:
    virtual ~Type() = default;

    Type(const Type&) = delete;
    void operator=(const Type&) = delete;

    Type(Type&&) noexcept = delete;
    void operator=(Type&&) noexcept = delete;

    /// Returns the string equivelant of this type.
    virtual std::string string() const = 0;

    /// Compare this type with |other| for type equality.
    virtual bool compare(const Type* other) const { return false; }

    /// Test if this type can be casted to |other|. 
    /// The |implicit| flag determines if casts should follow implicit rules.
    virtual bool can_cast(const Type* other, bool implicit = false) const {
        return false;
    }

    /// Test if this is the void type.
    virtual bool is_void() const { return false; }

    /// Test if this is an integer type of any signedness.
    virtual bool is_integer() const { return false; }

    /// Test if this is a signed integer type.
    virtual bool is_signed_integer() const { return false; }

    /// Test if this is an unsigned integer type.
    virtual bool is_unsigned_integer() const { return false; }

    /// Test if this is a floating point type.
    virtual bool is_floating_point() const { return false; }
};

/// Returns named type aliases defined by an alias definiiton.
class AliasType final : public Type {
    friend class Rib;

private:
    Type* m_aliased;
    AliasDefn* m_defn;

    AliasType(Type* aliased, AliasDefn* defn) 
      : m_aliased(aliased), m_defn(defn) {}

public:
    [[nodiscard]]
    static AliasType* create(Rib& rib, Type* aliased, AliasDefn* defn);
    
    [[nodiscard]]
    static AliasType* get(Rib& rib, const std::string& name);

    std::string string() const override;

    [[nodiscard]] bool compare(const Type* other) const override {
        return string() == other->string();
    }

    [[nodiscard]] 
    bool can_cast(const Type* other, bool implicit = false) const override;

    /// Set the type which this type aliases to |type|.
    void set_aliased(Type* type) { m_aliased = type; }

    /// Returns the type being aliased. 
    const Type* aliased() const { return m_aliased; }
    Type* aliased() { return m_aliased; }

    /// Set the definition which defines this alias to |defn|.
    void set_defn(AliasDefn* defn) { m_defn = defn; }

    /// Returns the definition which defines this alias.
    const AliasDefn* defn() const { return m_defn; }
    AliasDefn* defn() { return m_defn; }
};

/// Representation of types which are built-in to the language.
class BuiltinType final : public Type {
    friend class Rib;

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
    const Kind m_kind;

    BuiltinType(Kind kind) : m_kind(kind) {}

public:
    [[nodiscard]] static BuiltinType* get(Rib& rib, Kind kind);

    std::string string() const override;

    [[nodiscard]] 
    bool compare(const Type* other) const override;

    [[nodiscard]] 
    bool can_cast(const Type* other, bool implicit = false) const override;

    bool is_void() const override { return m_kind == Kind::Void; }

    bool is_integer() const override { 
        switch (m_kind) 
        {
        case Kind::Bool:
        case Kind::Char:
        case Kind::Int8:
        case Kind::Int16:
        case Kind::Int32:
        case Kind::Int64:
        case Kind::UInt8:
        case Kind::UInt16:
        case Kind::UInt32:
        case Kind::UInt64:
            return true;
        default:
            return false;
        }
    }

    bool is_signed_integer() const override { 
        switch (m_kind) 
        {
        case Kind::Bool:
        case Kind::Char:
        case Kind::Int8:
        case Kind::Int16:
        case Kind::Int32:
        case Kind::Int64:
            return true;
        default:
            return false;
        }
    }

    bool is_unsigned_integer() const override {
        switch (m_kind) 
        {
        case Kind::UInt8:
        case Kind::UInt16:
        case Kind::UInt32:
        case Kind::UInt64:
            return true;
        default:
            return false;
        }
    }

    bool is_floating_point() const override {
        return m_kind == Kind::Float32 || m_kind == Kind::Float64; 
    }

    /// Returns the kind of this built-in type.
    Kind kind() const { return m_kind; }
};

/// Wrapper class for types that were deferred resolution at parse time.
class DeferredType final : public Type {
    friend class Rib;

    std::string m_name;

    DeferredType(const std::string& name) : m_name(name) {}

public:
    static DeferredType* get(Rib& rib, const std::string& name);

    std::string string() const override { return std::format("'{}'", m_name); }

    /// Returns the name of this type.
    const std::string& name() const { return m_name; }
};

/// Represents named types defined by an enum definition.
class EnumType final : public Type {
    friend class Rib;

    Type* m_underlying;
    EnumDefn* m_defn;

    EnumType(Type* underlying, EnumDefn* defn) 
      : m_underlying(underlying), m_defn(defn) {}

public:
    [[nodiscard]]
    static EnumType* create(Rib& rib, Type* underlying, EnumDefn* defn);

    [[nodiscard]]
    static EnumType* get(Rib& rib, const std::string& name);

    std::string string() const override;

    [[nodiscard]] bool compare(const Type* other) const override {
        return string() == other->string();
    }

    [[nodiscard]] 
    bool can_cast(const Type* other, bool implicit = false) const override;

    /// Set the underlying numeric type of this enum to |type|.
    void set_underlying(Type* type) { m_underlying = type; }

    /// Returns the underlying numeric type of this enum.
    const Type* underlying() const { return m_underlying; }
    Type* underlying() { return m_underlying; }

    /// Set the definition which defines this enum to |defn|.
    void set_defn(EnumDefn* defn) { m_defn = defn; }

    /// Returns the definition which defines this enum..
    const EnumDefn* defn() const { return m_defn; }
    EnumDefn* defn() { return m_defn; }
};

/// Represents the type of a function signature i.e. a resulting type and a set 
/// of parameter types.
class FunctionType final : public Type {
    friend class Rib;
    
private:
    Type* m_result;
    std::vector<Type*> m_params;

    FunctionType(Type* result, const std::vector<Type*>& params) 
      : m_result(result), m_params(params) {}

public:
    [[nodiscard]]
    static FunctionType* get(Rib& rib, Type* result, 
                             const std::vector<Type*>& params);

    std::string string() const override;

    /// Returns the resulting type of this function type.
    const Type* result() const { return m_result; }
    Type* result() { return m_result; }

    /// Test if this function type has a result, i.e. does not result in 'void'.
    [[nodiscard]] bool has_result() const { return result()->is_void(); }

    /// Returns the parameter list of this function type.
    const std::vector<Type*>& params() const { return m_params; }
    std::vector<Type*>& params() { return m_params; }

    /// Returns the number of parameter types to this function type.
    uint32_t num_params() const { return m_params.size(); }

    /// Test if this function type has any parameter types.
    bool has_params() const { return !m_params.empty(); }

    /// Returns the |i|-th parameter type.
    const Type* get_param(uint32_t i) const {
        assert(i < m_params.size() && "index out of bounds!");
        return m_params[i];
    }

    Type* get_param(uint32_t i) {
        assert(i < m_params.size() && "index out of bounds!");
        return m_params[i];
    }
};

/// Represents composite pointer types.
class PointerType final : public Type {
    friend class Rib;

    Type* m_pointee;

    PointerType(Type* pointee) : m_pointee(pointee) {}

public:
    [[nodiscard]]
    static PointerType* get(Rib& rib, Type* pointee);

    std::string string() const override { 
        return std::format("*{}", m_pointee->string()); 
    }

    [[nodiscard]] 
    bool compare(const Type* other) const override;

    [[nodiscard]] 
    bool can_cast(const Type* other, bool implicit = false) const override;

    /// Set the pointee of this type to |type|.
    void set_pointee(Type* type) { m_pointee = type; }

    /// Returns the pointee type of this pointer type.
    const Type* pointee() const { return m_pointee; }
    Type* pointee() { return m_pointee; }
};

/// Represents named types defined by a struct definition.
class StructType final : public Type {
    friend class Rib;

    StructDefn* m_defn;

    StructType(StructDefn* defn) : m_defn(defn) {}

public:
    [[nodiscard]]
    static StructType* create(Rib& rib, StructDefn* defn);
    
    [[nodiscard]]
    static StructType* get(Rib& rib, const std::string& name);    

    std::string string() const override;

    [[nodiscard]] bool compare(const Type* other) const override { 
        return string() == other->string(); 
    }

    /// Set the definition which defines this structure type to |defn|.
    void set_defn(StructDefn* defn) { m_defn = defn; }
    
    /// Returns the definition which defines this structure type.
    const StructDefn* defn() const { return m_defn; }
    StructDefn* defn() { return m_defn; }
};

} // namespace lace

#endif // LACE_TYPE_H_
