#ifndef STATIM_TYPE_HPP_
#define STATIM_TYPE_HPP_

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

private:
    static id_t it;

public:
    Type() : tid(it++) {};

    virtual ~Type() = default;

    bool operator == (const Type& other) const {
        return tid == other.tid;
    }
};

/// Contextual properties for a type reference.
struct TypeContext final {
    std::string base;
    u32         indirection;
};

/// A deferred type, to be resolved after name resolution.
class DeferredType final : public Type {
    TypeContext context;
};

/// Represents a type built-in to the language.
class BuiltinType final : public Type {
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

public:
    static const BuiltinType* get(Root& root, Kind kind);

    ~BuiltinType() override = default;

    Kind get_kind() const { return kind; }
};

/// Represents the type defined by a function signature.
class FunctionType final : public Type {
    const Type*                 pReturn;
    std::vector<const Type*>    params;

public:
    static const FunctionType* get(
        Root& root, 
        const Type *pReturn, 
        const std::vector<const Type*>& params);
    
    ~FunctionType() override = default;

    const Type* get_return_type() const { return pReturn; }

    const Type* get_param_type(u32 idx) const {
        return params.at(idx);
    }

    const std::vector<const Type*>& get_param_types() const {
        return params;
    }
};

/// Represents the encapsulation of a type as a pointer.
class PointerType final : public Type {
    const Type* pPointee;

public:
    static const PointerType* get(Root& root, const Type* pPointee);

    ~PointerType() override = default;

    const Type* get_pointee() const { return pPointee; }
};

} // namespace stm

#endif // STATIM_TYPE_HPP_
