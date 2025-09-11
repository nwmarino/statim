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
class BuiltinType;
class FunctionType;
class ArrayType;
class PointerType;
class StructType;
class EnumType;

/// Base class for all types which may represent values in the syntax tree.
class Type {
protected:
    friend class Root;

    /// If true, then this type is considered mutable, which implicates
    /// certain semantics regarding writing to declarations considered 
    /// immutable.
    bool mut;

public:
    Type(bool mut = false) : mut(mut) {};

    virtual ~Type() = default;

    /// Returns true if this type is mutable.
    bool is_mut() const { return mut; }

    /// TODO: Implement this.
    /// Returns this type as a mutable variant.
    virtual const Type* as_mut() const { return is_mut() ? this : nullptr; }

    /// Returns true if this is the |void| type.
    virtual bool is_void() const { return false; }

    /// Returns true if this is the |bool| type.
    virtual bool is_bool() const { return false; }

    /// Returns true if this is the |char| type.
    virtual bool is_char() const { return false; }

    /// Returns true if this type can represent integers.
    virtual bool is_int() const { return false; }

    /// Returns true if and only if this type represents a signed integer.
    virtual bool is_signed_int() const { return false; }

    /// Returns true if and only if this type represents an unsigned integer.
    virtual bool is_unsigned_int() const { return false; }

    /// Returns true if and only if this type represent floating points values.
    virtual bool is_float() const { return false; }

    /// Returns true if this type has been deferred.
    constexpr virtual bool is_deferred() const { return false; }

    /// Returns this type as a deferred type, if it can be interpreted as one.
    virtual const DeferredType* as_deferred() const { 
        assert(false && "this type cannot be interpreted as a deferred type!"); 
    }

    /// Returns true if this type is a builtin type.
    constexpr virtual bool is_builtin() const { return false; }

    /// Returns this type as a builtin type, if it can be interpreted as one.
    virtual const BuiltinType* as_builtin() const { 
        assert(false && "this type cannot be interpreted as a builtin type!"); 
    }

    /// Returns true if this type is a function type.
    constexpr virtual bool is_function() const { return false; }

    /// Returns this type as a function type, if it can be interpreted as one.
    virtual const FunctionType* as_function() const {
        assert(false && "this type cannot be interpreted as a function type!"); 
    }

    /// Returns true if this type is an array type.
    constexpr virtual bool is_array() const { return false; }
    
    /// Returns this type as an array type, if it can be interpreted as one.
    virtual const ArrayType* as_array() const {
        assert(false && "this type cannot be interpreted as an array type!");
    }

    /// Returns true if this type is a pointer type.
    constexpr virtual bool is_pointer() const { return false; }

    /// Returns this type as a pointer type, if it can be interpreted as one.
    virtual const PointerType* as_pointer() const { 
        assert(false && "this type cannot be interpreted as a pointer type!");
    }

    /// Returns true if this type is a struct type.
    constexpr virtual bool is_struct() const { return false; }

    /// Returns this type as a struct type, if it can be interpreted as one.
    virtual const StructType* as_struct() const { 
        assert(false && "this type cannot be interpreted as a struct type!"); 
    }

    /// Returns true if this type is an enum type.
    constexpr virtual bool is_enum() const { return false; }

    /// Returns this type as an enum type, if it can be interpreted as one.
    virtual const EnumType* as_enum() const { 
        assert(false && "this type cannot be interpreted as a enum type!"); 
    }

    /// Returns true if this type is considered functionally equal to |other|.
    virtual bool compare(const Type* other) const { return false; }

    /// Returns true if this type can be casted to |other|. The |impl| flag
    /// determines if casting rules should fall under implicit casts or not.
    virtual bool can_cast(const Type* other, bool impl = false) const { 
        return false; 
    }

    /// Returns a string representation of this type.
    virtual std::string to_string() const = 0;
};

/// Represents types which have been deferred at parse-time. Instances of this
/// type live in the type context of the root in which they are declared, and
/// are to be resolved during name resolution.
///
/// Most actions on deferred types will fail by assertion if the underlying
/// type has yet to been resolved. Essentially, type operations should wait
/// until after validation of the type context to ensure that all deferred
/// types are resolved.
///
/// This primarily exists as a way to achieve forward references of types
/// without forward declarations, and to accomodate the cross-resolution of
/// type declarations amongst multiple source files.
class DeferredType final : public Type {
    friend class TypeContext;

public:
    /// Contextual properties for a type reference, resolved during parsing.
    struct Context final {
        /// TODO: Add context for array/pointer differentation, i.e. if a
        /// pointer to an array, add flag to signify.

        /// The base of this type, i.e. i8 in *i8, i32 in [4]i32, etc.
        std::string base;

        /// The location that this type was parsed.
        SourceLocation meta;

        /// If this type was marked as mutable or not.
        bool mut = false;

        /// The scope in which this type was referenced.
        const Scope* pScope;

        /// The size of this type if it was an array type, i.e. 4 in 4[i32].
        u32 size = 0;

        /// The level of indirection of this type if it is a pointer type, 
        /// i.e. if the type was parsed was **i32, then indirection = 2.
        u32 indirection = 0;
    };

private:
    Context m_context;

    /// The pointer to the resolved type, if it has been resolved.
    const Type* m_resolved = nullptr;

    /// Private constructor for the type context.
    DeferredType(const Context& context) : m_context(context) {};

public:
    /// Get a deferred type with properties |context|.
    static const DeferredType* get(Root& root, const Context& context);

    /// Returns contextual properties about this type.
    const Context& get_context() const { return m_context; }

    /// Returns the resolved type, if it exists yet.
    const Type* get_resolved() const { return m_resolved; }

    /// Returns true if the underlying type has been resolved yet.
    bool is_resolved() const { return m_resolved != nullptr; }

    /// Resolve this type as |type|.
    void set_resolved(const Type* ty) { m_resolved = ty; }

    /// Returns true if the underlying type is the |void| type.
    bool is_void() const override {
        assert(is_resolved() && "deferred type unresolved!");
        return m_resolved->is_void();
    }

    /// Returns true if the underlying type is the |bool| type.
    bool is_bool() const override {
        assert(is_resolved() && "deferred type unresolved!");
        return m_resolved->is_bool();
    }

    /// Returns true if the underlying type is the |char| type.
    bool is_char() const override {
        assert(is_resolved() && "deferred type unresolved!");
        return m_resolved->is_char();
    }
    
    bool is_int() const override {
        assert(is_resolved() && "deferred type unresolved!");
        return m_resolved->is_int();
    }

    bool is_signed_int() const override {
        assert(is_resolved() && "deferred type unresolved!");
        return m_resolved->is_signed_int();
    }

    bool is_unsigned_int() const override {
        assert(is_resolved() && "deferred type unresolved!");
        return m_resolved->is_unsigned_int();
    }
    
    bool is_float() const override {
        assert(is_resolved() && "deferred type unresolved!");
        return m_resolved->is_float();
    }

    constexpr bool is_deferred() const override { return true; }

    const DeferredType* as_deferred() const override { return this; }

    constexpr bool is_pointer() const override {
        assert(is_resolved() && "deferred type unresolved!");
        return m_resolved->is_pointer(); 
    }

    const PointerType* as_pointer() const override {
        assert(is_resolved() && "deferred type unresolved!");
        return m_resolved->as_pointer(); 
    }

    constexpr bool is_struct() const override {
        assert(is_resolved() && "deferred type unresolved!");
        return m_resolved->is_struct(); 
    }

    const StructType* as_struct() const override {
        assert(is_resolved() && "deferred type unresolved!");
        return m_resolved->as_struct(); 
    }

    constexpr bool is_enum() const override {
        assert(is_resolved() && "deferred type unresolved!");
        return m_resolved->is_enum(); 
    }

    const EnumType* as_enum() const override {
        assert(is_resolved() && "deferred type unresolved!");
        return m_resolved->as_enum(); 
    }

    bool compare(const Type* other) const override {
        assert(is_resolved() && "deferred type unresolved!");
        return m_resolved->compare(other); 
    }

    bool can_cast(const Type* other, bool impl = false) const override {
        assert(is_resolved() && "deferred type unresolved!");
        return m_resolved->can_cast(other, impl); 
    }

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
    Kind m_kind;

    /// Private constructor for the type context.
    BuiltinType(Kind kind) : m_kind(kind) {}

public:
    /// Get the built-in type of the given kind.
    static const BuiltinType* get(Root& root, Kind kind);

    /// Returns the kind of this built-in type.
    Kind kind() const { return m_kind; }

    bool is_void() const override { return kind() == Kind::Void; }

    bool is_bool() const override { return kind() == Kind::Bool; }

    bool is_char() const override { return kind() == Kind::Char; }

    bool is_int() const override {
        return Kind::Bool <= kind() && kind() <= Kind::UInt64;
    }

    bool is_signed_int() const override {
        return Kind::SInt8 <= kind() && kind() <= Kind::SInt64;
    }

    bool is_unsigned_int() const override {
        return Kind::UInt8 <= kind() && kind() <= Kind::UInt64;
    }

    bool is_float() const override {
        return kind() == Kind::Float32 || kind() == Kind::Float64;
    }

    constexpr bool is_builtin() const override { return true; }

    const BuiltinType* as_builtin() const override { return this; }

    bool compare(const Type* other) const override;

    bool can_cast(const Type* other, bool impl = false) const override;

    std::string to_string() const override;
};

/// Represents the type defined by a function signature.
class FunctionType final : public Type {
    friend class TypeContext;

    /// The resulting type of calls to functions with this type.
    const Type* m_ret;
    
    /// The types of arguments given to calls to functions with this type.
    std::vector<const Type*> m_params;

    /// Private constructor for the type context.
    FunctionType(const Type* ret, const std::vector<const Type*>& params)
        : Type(false), m_ret(ret), m_params(params) {}

public:
    /// Get the function type with return type |ret| and parameter list 
    /// |params|.
    static const FunctionType* get(Root& root, const Type *ret, 
                                   const std::vector<const Type*>& params);

    /// Returns the return type of this function signature type.
    const Type* get_return_type() const { return m_ret; }

    /// Returns the type of a parameter at position |idx| of this function 
    /// signature type.
    const Type* get_param_type(u32 idx) const { return m_params.at(idx); }

    /// Returns all the parameter types of this function signature type.
    const std::vector<const Type*>& get_param_types() const { 
        return m_params; 
    }

    /// Returns the number of parameters in this function signature type.
    u32 num_params() const { return get_param_types().size(); }

    constexpr bool is_function() const override { return true; }

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
    
    /// The pointee type of this pointer, i.e. the type of the value which
    /// pointers this type represents point to.
    const Type* m_pointee;

    /// Private constructor for the type context.
    PointerType(const Type* pointee) : m_pointee(pointee) {}

public:
    /// Get the pointer type that encapsulates |pointee|.
    static const PointerType* get(Root& root, const Type* pointee);

    /// Returns the pointee of this pointer type.
    const Type* get_pointee() const { return m_pointee; }

    /// Returns the level of indirection of this pointer type.
    u32 get_indirection() const;

    constexpr bool is_pointer() const override { return true; }

    const PointerType* as_pointer() const override { return this; }

    bool compare(const Type* other) const override;

    bool can_cast(const Type* other, bool impl = false) const override;

    std::string to_string() const override;
};

/// Represents the type defined by a struct declaration.
class StructType final : public Type {
    friend class TypeContext;

    /// The types of fields in this structure.
    std::vector<const Type*> m_fields;

    /// The corresponding node that defines this type.
    const StructDecl* m_decl;

    /// Private constructor for the type context.
    StructType(const std::vector<const Type*>& fields, const StructDecl* decl)
        : m_fields(fields), m_decl(decl) {};

public:
    /// Get an existing struct type by name, if one exists.
    static const StructType* get(Root& root, const std::string& name);

    /// Create a new struct type with the given field types.
    static const StructType* create(
        Root& root, 
        const std::vector<const Type*>& fields, 
        const StructDecl* pDecl);

    /// Returns the types of all fields in this struct type.
    const std::vector<const Type*>& get_fields() const { return m_fields; }

    /// Returns the number of fields in this structure.
    u32 num_fields() const { return m_fields.size(); }

    /// Returns the declaration that defines this struct type.
    const StructDecl* get_decl() const { return m_decl; }

    constexpr bool is_struct() const override { return true; }

    const StructType* as_struct() const override { return this; }

    bool compare(const Type* other) const override;

    std::string to_string() const override;
};

/// Represents the type defined by an enum declaration.
class EnumType final : public Type {
    friend class TypeContext;

    /// The underlying type of this enumeration type.
    const Type* m_underlying;

    /// The corresponding node that defines this type.
    const EnumDecl* m_decl;
    
    /// Private constructor for the type context.
    EnumType(const Type* underlying, const EnumDecl* decl)
        : m_underlying(underlying), m_decl(decl) {};

public:
    /// Get an existing enum type by name, if one exists.
    static const EnumType* get(Root& root, const std::string& name);

    /// Create a new enum type with the given underlying type.
    static const EnumType* create(Root& root, const Type* underlying,
                                  const EnumDecl* decl);

    /// Returns the underlying integer type of this enum type.
    const Type* get_underlying() const { return m_underlying; }

    /// Returns the declaration that defines this type.
    const EnumDecl* get_decl() const { return m_decl; }

    bool is_int() const override { return true; }

    constexpr bool is_enum() const override { return true; }

    const EnumType* as_enum() const override { return this; }

    bool compare(const Type* other) const override;

    std::string to_string() const override;
};

} // namespace stm

#endif // STATIM_TREE_TYPE_HPP_
