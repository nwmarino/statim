#ifndef STATIM_TYPE_HPP_
#define STATIM_TYPE_HPP_

#include "scope.hpp"
#include "source_loc.hpp"
#include "types.hpp"

#include <string>
#include <vector>

namespace stm {

class Root;

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

    bool is_mut() const { return mut; }

    virtual bool is_void() const { return false; }

    virtual bool is_int() const { return false; }

    virtual bool is_float() const { return false; }

    virtual bool can_cast(const Type* other, bool impl = false) const
    { return false; }

    virtual std::string to_string() const = 0;
};

/// A deferred type, to be resolved after name resolution.
class DeferredType final : public Type {
    friend class TypeContext;

public:
    /// Contextual properties for a type reference.
    struct Context final {
        std::string     base;
        SourceLocation  meta;
        bool            mut;
        const Scope*    pScope;
        u32             indirection;
    };

private:
    Context     context;
    const Type* pResolved;

    DeferredType(const Context& context) 
        : context(context), pResolved(nullptr) {};

public:
    static const DeferredType* get(Root& root, const Context& context);

    const Context& get_context() const { return context; }

    const Type* get_resolved() const { return pResolved; }

    void set_resolved(const Type* pType) { pResolved = pType; }

    bool is_int() const override;

    bool is_float() const override;

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

    /// Get the name as it is reserved in the language by \p kind.
    static std::string get_name(Kind kind);

private:
    Kind kind;

    BuiltinType(Kind kind) : kind(kind) {};

public:
    static const BuiltinType* get(Root& root, Kind kind);

    Kind get_kind() const { return kind; }

    bool is_void() const override { return kind == Kind::Void; }

    bool is_int() const override;

    bool is_float() const override;

    bool can_cast(const Type* other, bool impl = false) const override;

    std::string to_string() const override;
};

/// Represents the type defined by a function signature.
class FunctionType final : public Type {
    friend class TypeContext;

    const Type*                 pReturn;
    std::vector<const Type*>    params;

    FunctionType(const Type* pReturn, const std::vector<const Type*>& params);

public:
    static const FunctionType* get(
        Root& root, 
        const Type *pReturn, 
        const std::vector<const Type*>& params);

    const Type* get_return_type() const { return pReturn; }

    const Type* get_param_type(u32 idx) const {
        return params.at(idx);
    }

    const std::vector<const Type*>& get_param_types() const {
        return params;
    }

    std::string to_string() const override;
};

/// Represents the encapsulation of a type as a pointer.
class PointerType final : public Type {
    friend class TypeContext;
    
    const Type* pPointee;

    PointerType(const Type* pPointee) : pPointee(pPointee) {};

public:
    static const PointerType* get(Root& root, const Type* pPointee);

    const Type* get_pointee() const { return pPointee; }

    /// Get the level of indirection of this pointer type.
    u32 get_indirection() const;

    bool can_cast(const Type* other, bool impl = false) const override;

    std::string to_string() const override;
};

} // namespace stm

#endif // STATIM_TYPE_HPP_
