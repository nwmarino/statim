#ifndef STATIM_TREE_TYPE_HPP_
#define STATIM_TREE_TYPE_HPP_

#include "tree/scope.hpp"
#include "types/source_location.hpp"
#include "types/types.hpp"

#include <cassert>
#include <string>
#include <vector>

namespace stm {

class Root;
class StructDecl;
class EnumDecl;

class DeferredType;
class FunctionType;
class ArrayType;
class PointerType;
class StructType;
class EnumType;

/// Base class for all frontend types.
class Type {
protected:
    friend class Root;

    using id_t = u32;

    id_t tid;
    bool mut;

private:
    static id_t it;

public:
    Type(bool mut = false) : tid(it++), mut(mut) {};

    virtual ~Type() = default;

    bool operator == (const Type& other) const {
        return tid == other.tid;
    }

    /// \returns `true` if this type is mutable.
    bool is_mut() const { return mut; }

    /// \returns `true` if this is the `void` type.
    virtual bool is_void() const { return false; }

    /// \returns `true` if this is the `bool` type.
    virtual bool is_bool() const { return false; }

    /// \returns `true` if this type can represent integers.
    virtual bool is_int() const { return false; }

    /// \returns `true` only if this type is a signed integer.
    virtual bool is_signed_int() const { return false; }

    /// \returns `true` only if this type is an unsigned integer.
    virtual bool is_unsigned_int() const { return false; }

    /// \returns `true` if this type can represent floating points.
    virtual bool is_float() const { return false; }

    /// \returns `true` if this type is a deferred type.
    virtual bool is_deferred() const { return false; }

    /// \returns This type as a defered type, if it is one.
    virtual const DeferredType* as_deferred() const
    { assert(false && "cannot convert this type to a deferred type"); }

    /// \returns `true` if this type is a function type.
    virtual bool is_function() const { return false; }

    /// \returns This type as a function type, if it is one.
    virtual const FunctionType* as_function() const
    { assert(false && "cannot convert this type to a function type"); }

    /// Returns true if this type is an array type.
    virtual bool is_array() const { return false; }
    
    /// Returns this type as an array type, if it is one.
    virtual const ArrayType* as_array() const {
        assert(false && "cannot convert this type to an array type!");
    }

    /// \returns `true` if this type is a pointer type.
    virtual bool is_pointer() const { return false; }

    /// \returns This type as a pointer type, if it is one.
    virtual const PointerType* as_pointer() const
    { assert(false && "cannot convert this type to a pointer type"); }

    /// \returns `true` if this type is a struct type.
    virtual bool is_struct() const { return false; }

    /// \returns This type as a struct type, if it is one.
    virtual const StructType* as_struct() const
    { assert(false && "cannot convert this type to a struct type"); }

    /// \returns `true` if this type is an enum type.
    virtual bool is_enum() const { return false; }

    /// \returns This type as an enum type, if it is one.
    virtual const EnumType* as_enum() const
    { assert(false && "cannot convert this type to an enum type"); }

    /// \returns `true` if this type can be casted to the \p other type.
    virtual bool can_cast(const Type* other, bool impl = false) const
    { return false; }

    /// \returns A string representation of this type.
    virtual std::string to_string() const = 0;
};

/// A deferred type, to be resolved after name resolution.
class DeferredType final : public Type {
    friend class TypeContext;

public:
    /// Contextual properties for a type reference.
    struct Context final {
        /// The base of this type, i.e. i8 in *i8, i32 in [4]i32, etc.
        std::string base;

        /// The location at with this type was parsed.
        SourceLocation meta;

        /// If this type was marked as mutable or not.
        bool mut;

        /// The scope in which this type was referenced.
        const Scope* pScope;

        /// The size of this type if it was an array type, i.e. 4 in 4[i32].
        u32 size = 0;

        /// The level of indirection if this type was a pointer type, i.e. if
        /// the type was parsed was **i32, then indirection = 2.
        u32 indirection = 0;
    };

private:
    Context context;
    const Type* pResolved = nullptr;

    /// Private constructor for the type context.
    DeferredType(const Context& context) : context(context) {};

public:
    /// Get a deferred type with the provided \p context.
    static const DeferredType* get(Root& root, const Context& context);

    /// \returns The context in which this type was deferred.
    const Context& get_context() const { return context; }

    /// \returns The resolved type, if it exists.
    const Type* get_resolved() const { return pResolved; }

    /// Resolve this type as type \p pType.
    void set_resolved(const Type* pType) { pResolved = pType; }

    bool is_void() const override;

    bool is_bool() const override;
    
    bool is_int() const override;

    bool is_signed_int() const override;

    bool is_unsigned_int() const override;
    
    bool is_float() const override;

    bool is_deferred() const override { return true; }

    const DeferredType* as_deferred() const override { return this; }

    bool is_pointer() const override 
    { return pResolved && pResolved->is_pointer(); }

    const PointerType* as_pointer() const override;

    bool is_struct() const override
    { return pResolved && pResolved->is_struct(); }

    const StructType* as_struct() const override;

    bool is_enum() const override
    { return pResolved && pResolved->is_enum(); }

    const EnumType* as_enum() const override;

    bool can_cast(const Type* other, bool impl = false) const override;

    std::string to_string() const override;
};

/// Represents a type built-in to the language.
class BuiltinType final : public Type {
    friend class TypeContext;

public:
    /// Recognized built-in types.
    enum class Kind : u8 {
        Void, Bool, Char,
        SInt8, SInt16, SInt32, SInt64,
        UInt8, UInt16, UInt32, UInt64,
        Float32, Float64,
    };

    /// \returns The name of a built-in type as it is reserved in the language.
    static std::string get_name(Kind kind);

private:
    Kind kind;

    /// Private constructor for the type context.
    BuiltinType(Kind kind) : kind(kind) {};

public:
    /// Get the built-in type of the given \p kind.
    static const BuiltinType* get(Root& root, Kind kind);

    /// \returns The kind of this built-in type.
    Kind get_kind() const { return kind; }

    bool is_void() const override { return kind == Kind::Void; }

    bool is_bool() const override { return kind == Kind::Bool; }

    bool is_int() const override;

    bool is_signed_int() const override;

    bool is_unsigned_int() const override;

    bool is_float() const override;

    bool can_cast(const Type* other, bool impl = false) const override;

    std::string to_string() const override;
};

/// Represents the type defined by a function signature.
class FunctionType final : public Type {
    friend class TypeContext;

    const Type*                 pReturn;
    std::vector<const Type*>    params;

    /// Private constructor for the type context.
    FunctionType(const Type* pReturn, const std::vector<const Type*>& params);

public:
    /// Get the function type with return type \p pReturn and parameter types
    /// \p params.
    static const FunctionType* get(
        Root& root, 
        const Type *pReturn, 
        const std::vector<const Type*>& params);

    /// \returns The return type of this function signature.
    const Type* get_return_type() const { return pReturn; }

    /// \returns The type of a parameter of this function signature.
    const Type* get_param_type(u32 idx) const { return params.at(idx); }

    /// \returns All the parameter types of this function signature.
    const std::vector<const Type*>& get_param_types() const { return params; }

    bool is_function() const override { return true; }

    const FunctionType* as_function() const override { return this; }

    std::string to_string() const override;
};

/// An array type - used for statically sized aggregates whose elements are
/// the same type.
/*
class ArrayType final : public Type {
    friend class TypeContext;

    const Type* m_element;
    u32 m_size;

    ArrayType(const Type* element, u32 size) 
        : m_element(element), m_size(size) {}

public:
    static const ArrayType* get(Root& root, const Type* element, u32 size);

    /// Returns the element type of this array type.
    const Type* get_element() const { return m_element; }

    /// Returns the number of elements in this array type.
    u32 get_size() const { return m_size; }

    bool is_array() const override { return true; }

    const ArrayType* as_array() const override { return this; }

    bool can_cast(const Type* other, bool impl = false) const override;

    std::string to_string() const override;
};
*/

/// Represents the encapsulation of a type as a pointer.
class PointerType final : public Type {
    friend class TypeContext;
    
    const Type* pPointee;

    /// Private constructor for the type context.
    PointerType(const Type* pPointee) : pPointee(pPointee) {};

public:
    /// Get the pointer type with pointee \p pPointee.
    static const PointerType* get(Root& root, const Type* pPointee);

    /// \returns The pointee of this pointer type.
    const Type* get_pointee() const { return pPointee; }

    /// \returns The level of indirection of this pointer type.
    u32 get_indirection() const;

    bool is_pointer() const override { return true; }

    const PointerType* as_pointer() const override { return this; }

    bool can_cast(const Type* other, bool impl = false) const override;

    std::string to_string() const override;
};

/// Represents the type defined by a struct declaration.
class StructType final : public Type {
    friend class TypeContext;

    std::vector<const Type*> m_fields;
    const StructDecl* m_decl;

    /// Private constructor for the type context.
    StructType(const std::vector<const Type*>& fields, const StructDecl* decl)
        : m_fields(fields), m_decl(decl) {};

public:
    /// Get an existing struct type by name, if one exists.
    static const StructType* get(Root& root, const std::string& name);

    /// Create a new struct type with the given field types \p fields.
    static const StructType* create(
        Root& root, 
        const std::vector<const Type*>& fields, 
        const StructDecl* pDecl);

    /// \returns The field types of this struct type.
    const std::vector<const Type*>& get_fields() const { return m_fields; }

    /// \returns The number of fields in this structure.
    u32 num_fields() const { return m_fields.size(); }

    /// \returns The declaration that defines this struct type.
    const StructDecl* get_decl() const { return m_decl; }

    bool is_struct() const override { return true; }

    const StructType* as_struct() const override { return this; }

    std::string to_string() const override;
};

/// Represents the type defined by an enum declaration.
class EnumType final : public Type {
    friend class TypeContext;

    const Type* m_underlying;
    const EnumDecl* m_decl;
    
    /// Private constructor for the type context.
    EnumType(const Type* underlying, const EnumDecl* decl)
        : m_underlying(underlying), m_decl(decl) {};

public:
    /// Get an existing enum type by name, if one exists.
    static const EnumType* get(Root& root, const std::string& name);

    /// Create a new enum type with the underlying \p underlying type.
    static const EnumType* create(
        Root& root,
        const Type* underlying,
        const EnumDecl* decl);

    /// \returns The underlying integer type of this enum type.
    const Type* get_underlying() const { return m_underlying; }

    /// \returns The declaration that defines this type.
    const EnumDecl* get_decl() const { return m_decl; }

    bool is_int() const override { return true; }

    bool is_enum() const override { return true; }

    const EnumType* as_enum() const override { return this; }

    std::string to_string() const override;
};

} // namespace stm

#endif // STATIM_TREE_TYPE_HPP_
