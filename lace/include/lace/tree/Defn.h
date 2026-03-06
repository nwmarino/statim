//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LACE_DEFN_H_
#define LACE_DEFN_H_

//
//  This header file declares a set of polymorphic classes for representing 
//  language definitions in the abstract syntax tree.
//

#include "lace/tree/Rib.h"
#include "lace/tree/Rune.h"
#include "lace/tree/Type.h"
#include "lace/tree/VisitorBase.h"
#include "lace/types/SourceSpan.h"

#include <cassert>
#include <string>
#include <vector>

namespace lace {

class Expr;
class Stmt;
class BlockStmt;
class Rune;
class Scope;
class Type;

/// Base class for all definition types in the abstract syntax tree.
class Defn {
protected:
    /// The parent rib of this definition.
    Rib* m_rib;

    /// The span of source code that this definition covers.
    SourceSpan m_span;

    Defn(Rib* rib, SourceSpan span) : m_rib(rib), m_span(span) {}

public:
    virtual ~Defn() = default;

    Defn(const Defn&) = delete;
    void operator=(const Defn&) = delete;

    Defn(Defn&&) noexcept = delete;
    void operator=(Defn&&) noexcept = delete;

    virtual void accept(VisitorBase& visitor) = 0;

    /// Returns the originating rib of this definition.
    const Rib* rib() const { return m_rib; }
    Rib* rib() { return m_rib; }

    /// Set the span of source code which this definition covers to |defn|.
    void set_span(SourceSpan span) { m_span = span; }

    /// Returns the span of source code which this definition covers.
    const SourceSpan& span() const { return m_span; }
    SourceSpan& span() { return m_span; }
};

/// Represents a top-level `use` definition.
class UseDefn final : public Defn {
    std::string m_path;
    Rib* m_target = nullptr;

    UseDefn(Rib* rib, SourceSpan span, const std::string& path) 
      : Defn(rib, span), m_path(path) {}

public:
    [[nodiscard]]
    static UseDefn* create(Rib& rib, SourceSpan span, const std::string& trail);

    ~UseDefn() = default;

    UseDefn(const UseDefn&) = delete;
    void operator=(const UseDefn&) = delete;

    UseDefn(UseDefn&&) noexcept = delete;
    void operator=(UseDefn&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { visitor.visit(*this); }

    /// Returns the rib path which this use targets.
    const std::string& path() const { return m_path; }
    std::string& path() { return m_path; }

    /// Set the target rib of this use to the given |rib|.
    void set_target(Rib* rib) { m_target = rib; }

    /// Returns the target rib of this use.
    const Rib* target() const { return m_target; }
    Rib* target() { return m_target; }
};

/// Base class for definitions with a name and potential rune set.
class NamedDefn : public Defn {
public:
    using Runes = std::vector<Rune*>;

protected:
    std::string m_name;
    Runes m_runes;

    NamedDefn(Rib* rib, SourceSpan span, const std::string& name, 
              const Runes& runes)
      : Defn(rib, span), m_name(name), m_runes(runes) {}

public:
    virtual ~NamedDefn() override;

    NamedDefn(const NamedDefn&) = delete;
    void operator=(const NamedDefn&) = delete;

    NamedDefn(NamedDefn&&) noexcept = delete;
    void operator=(NamedDefn&&) noexcept = delete;

    /// Sets the name of this definition to |name|.
    void set_name(const std::string& name) { m_name = name; }

    /// Returns the name of this definition.
    const std::string& name() const { return m_name; }
    std::string& name() { return m_name; }

    /// Returns the rune decorator list of this definiton.
    const Runes& runes() const { return m_runes; }
    Runes& runes() { return m_runes; }

    /// Adds the given |rune| as a decorator to this definition, if it isn't
    /// already in the list of active runes.
    void add_rune(Rune* rune) {
        if (!has_rune(rune->kind()))
            m_runes.push_back(rune);
    }

    /// Returns the rune with the given |kind| if this definition has one, and 
    /// null otherwise.
    const Rune* get_rune(Rune::Kind kind) const {
        for (Rune* rune : m_runes) {
            if (rune->has_kind(kind))
                return rune;
        }

        return nullptr;
    }

    Rune* get_rune(Rune::Kind kind) {
        return const_cast<Rune*>(
            static_cast<const NamedDefn*>(this)->get_rune(kind));
    }

    /// Returns the |i|-th decorator rune of this definition.
    const Rune* get_rune(uint32_t i) const {
        assert(i < m_runes.size() && "index out of bounds!");
        return m_runes[i];
    }

    Rune* get_rune(uint32_t i) {
        assert(i < m_runes.size() && "index out of bounds!");
        return m_runes[i];
    }
    
    /// Test if this definition has a rune of the given |kind|.
    bool has_rune(Rune::Kind kind) const {
        for (Rune* rune : m_runes) {
            if (rune->has_kind(kind))
                return true;
        }
            
        return false;
    }

    /// Returns the number of decorator runes this definition has.
    uint32_t num_runes() const { return m_runes.size(); }

    /// Test if this definition has any decorator runes.
    bool has_runes() const { return !m_runes.empty(); }
};

/// Base class for all named definitions that are typed and produce a value.
class ValueDefn : public NamedDefn {
protected:
    Type* m_type;

    ValueDefn(Rib* rib, SourceSpan span, const std::string& name, 
              const Runes& runes, Type* type)
      : NamedDefn(rib, span, name, runes), m_type(type) {}

public:
    virtual ~ValueDefn() = default;

    ValueDefn(const ValueDefn&) = delete;
    void operator=(const ValueDefn&) = delete;

    ValueDefn(ValueDefn&&) noexcept = delete;
    void operator=(ValueDefn&&) noexcept = delete;

    /// Set the type of this definition to |type|.
    void set_type(Type* type) { m_type = type; }

    /// Returns the type of this definition.
    const Type* type() const { return m_type; }
    Type* type() { return m_type; }
};

/// Represents a variable definition, either local or global.
class VariableDefn final : public ValueDefn {
    friend class SemanticAnalysis;

    // The initializing expression of this variable, if there is one.
    Expr* m_init;

    // If this is a global variable.
    bool m_global;

    VariableDefn(Rib* rib, SourceSpan span, const std::string& name, 
                 const Runes& runes, Type* type, Expr* init, bool global)
      : ValueDefn(rib, span, name, runes, type), m_init(init), 
        m_global(global) {}

public:
    [[nodiscard]]
    static VariableDefn* create(Rib& rib, SourceSpan span, 
                                const std::string& name, const Runes& runes, 
                                Type* type, Expr* init, bool global);

    ~VariableDefn() override;

    VariableDefn(const VariableDefn&) = delete;
    void operator=(const VariableDefn&) = delete;

    VariableDefn(VariableDefn&&) noexcept = delete;
    void operator=(VariableDefn&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { visitor.visit(*this); }

    /// Returns the initializing expression of this variable, if it has one,
    /// and null otherwise.
    const Expr* init() const { return m_init; }
    Expr* init() { return m_init; }

    /// Test if this variable has an initializing expression.
    bool has_init() const { return m_init != nullptr; }

    /// Test if this variable is global i.e. top-level.
    bool is_global() const { return m_global; }
};

/// Represents a function parameter definition.
class ParameterDefn final : public ValueDefn {
    ParameterDefn(Rib* rib, SourceSpan span, const std::string& name, 
                  const Runes& runes, Type* type)
      : ValueDefn(rib, span, name, runes, type) {}

public:
    [[nodiscard]]
    static ParameterDefn* create(Rib& rib, SourceSpan span, 
                                 const std::string& name, const Runes& runes, 
                                 Type* type);

    ~ParameterDefn() = default;

    ParameterDefn(const ParameterDefn&) = delete;
    void operator=(const ParameterDefn&) = delete;

    ParameterDefn(ParameterDefn&&) noexcept = delete;
    void operator=(ParameterDefn&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { visitor.visit(*this); }
};

/// Represents a function definiiton.
class FunctionDefn final : public ValueDefn {
public:
    using Params = std::vector<ParameterDefn*>;

private:
    /// The scope of this function.
    ///
    /// This scope tree is a different node than the scope of the function body. 
    /// This scope contains named definitions coupled directly with the 
    /// function i.e. named parameters.
    Scope* m_scope;

    /// The pointer receiver of this function, if it has one.
    ///
    /// Pointer receivers allow functions to be recognized as methods for a
    /// given type. They are effectively specialized parameters.
    ParameterDefn* m_receiver;

    /// The list of parameters for this function.
    Params m_params;

    /// The body of the function, if it has one.
    BlockStmt* m_body;

    FunctionDefn(Rib* rib, SourceSpan span, const std::string& name, 
                 const Runes& runes, FunctionType* type, Scope* scope, 
                 ParameterDefn* receiver, const Params& params, BlockStmt* body)
      : ValueDefn(rib, span, name, runes, type), m_scope(scope), 
        m_receiver(receiver), m_params(params), m_body(body) {}

public:
    [[nodiscard]]
    static FunctionDefn* create(Rib& rib, SourceSpan span, 
                                const std::string& name, const Runes& runes, 
                                FunctionType* type, Scope* scope, 
                                ParameterDefn* receiver, const Params& params, 
                                BlockStmt* body = nullptr);

    ~FunctionDefn() override;

    FunctionDefn(const FunctionDefn&) = delete;
    void operator=(const FunctionDefn&) = delete;

    FunctionDefn(FunctionDefn&&) noexcept = delete;
    void operator=(FunctionDefn&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { visitor.visit(*this); }

    /// Set the type of this definition to |type|.
    void set_type(FunctionType* type) { m_type = type; }

    /// Returns the type of this definition.
    const FunctionType* type() const { return static_cast<const FunctionType*>(m_type); }
    FunctionType* type() { return static_cast<FunctionType*>(m_type); }

    /// Test if this is the main function i.e. a function named `main`.
    bool is_main() const { return name() == "main"; }

    /// Returns the type which this function results in.
    const Type* get_return_type() const {
        return static_cast<const FunctionType*>(m_type)->result();
    }

    Type* get_return_type() {
        return static_cast<FunctionType*>(m_type)->result();
    }

    /// Returns the scope tree of this function.
    const Scope* scope() const { return m_scope; }
    Scope* scope() { return m_scope; }

    /// Returns the receiver of this function, if it has one.
    const ParameterDefn* receiver() const { return m_receiver; }
    ParameterDefn* receiver() { return m_receiver; }

    /// Test if this function has a receiver.
    bool has_receiver() const { return m_receiver != nullptr; }

    /// Returns the value type of the receiver which this function acts as a
    /// method for.
    /// If this function does not have a receiver, null is returned.
    const Type* get_receiver_type() const;
    Type* get_receiver_type() {
        return const_cast<Type*>(
            static_cast<const FunctionDefn*>(this)->get_receiver_type());
    }

    /// Set the parameter list of this function to |params|.
    void set_params(const Params& params) { m_params = params; }

    /// Returns the parameter list of this function.
    const Params& params() const { return m_params; }
    Params& params() { return m_params; }

    /// Returns the |i|-th parameter of this function.
    const ParameterDefn* get_param(uint32_t i) const {
        assert(i < m_params.size() && "index out of bounds!");
        return m_params[i];
    }

    ParameterDefn* get_param(uint32_t i) {
        assert(i < m_params.size() && "index out of bounds!");
        return m_params[i];
    }

    /// Returns the number of parameters in this function.
    uint32_t num_params() const { return m_params.size(); }

    /// Test if this function has any parameters.
    bool has_params() const { return !m_params.empty(); }

    /// Set the body statement of this function to |body|.
    void set_body(BlockStmt* body) { m_body = body; }

    /// Returns the body statement of this function.
    const BlockStmt* body() const { return m_body; }
    BlockStmt* body() { return m_body; }

    /// Test if this statement has a body.
    bool has_body() const { return m_body != nullptr; }

    /// Test if this function is empty i.e. does not have a body statement.
    [[nodiscard]] bool empty() const { return m_body == nullptr; }
};

/// Represents a field definition within a structure.
class FieldDefn final : public ValueDefn {
    friend class SemanticAnalysis;

    Expr* m_init;
    uint32_t m_index;

    FieldDefn(Rib* rib, SourceSpan span, const std::string& name, 
              const Runes& runes, Type* type, Expr* init, uint32_t index)
      : ValueDefn(rib, span, name, runes, type), m_init(init), m_index(index) {}

public:
    [[nodiscard]]
    static FieldDefn* create(Rib& rib, SourceSpan span, 
                             const std::string& name, const Runes& runes, 
                             Type* type, Expr* init, uint32_t index);

    ~FieldDefn() override;

    FieldDefn(const FieldDefn&) = delete;
    void operator=(const FieldDefn&) = delete;

    FieldDefn(FieldDefn&&) noexcept = delete;
    void operator=(FieldDefn&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { visitor.visit(*this); }

    /// Returns the default initalizer for this field, if it has one, and null
    /// otherwise.
    const Expr* init() const { return m_init; }
    Expr* init() { return m_init; }

    /// Test if this field has a default initializer.
    bool has_init() const { return m_init != nullptr; }

    /// Returns the index of this field in its parent structure.
    uint32_t get_index() const { return m_index; }
};

/// Represents an enum variant definition.
class VariantDefn final : public ValueDefn {
    int64_t m_value;

    VariantDefn(Rib* rib, SourceSpan span, const std::string& name, 
                const Runes& runes, Type* type, int64_t value)
      : ValueDefn(rib, span, name, runes, type), m_value(value) {}

public:
    [[nodiscard]]
    static VariantDefn* create(Rib& rib, SourceSpan span, 
                               const std::string& name, const Runes& runes, 
                               Type* type, int64_t value);

    ~VariantDefn() = default;

    VariantDefn(const VariantDefn&) = delete;
    void operator=(const VariantDefn&) = delete;

    VariantDefn(VariantDefn&&) noexcept = delete;
    void operator=(VariantDefn&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { visitor.visit(*this); }
    
    /// Returns the numeric value of this enum variant.
    int64_t get_value() const { return m_value; }
};

/// Base class for all named definitions that define a new type.
class TypeDefn : public NamedDefn {
protected:
    /// The type which this definition defines.
    Type* m_type;

    TypeDefn(Rib* rib, SourceSpan span, const std::string& name, 
             const Runes& runes, Type* type)
      : NamedDefn(rib, span, name, runes), m_type(type) {}

public:
    virtual ~TypeDefn() = default;

    TypeDefn(const TypeDefn&) = delete;
    void operator=(const TypeDefn&) = delete;

    TypeDefn(TypeDefn&&) noexcept = delete;
    void operator=(TypeDefn&&) noexcept = delete;

    /// Set the type which this definition defines to |type|.
    void set_type(Type* type) { m_type = type; }

    /// Returns the type which this definition defines.
    const Type* type() const { return m_type; }
    Type* type() { return m_type; }
};

/// Represents a type alias definition.
class AliasDefn final : public TypeDefn {
    AliasDefn(Rib* rib, SourceSpan span, const std::string& name, 
              const Runes& runes, Type* type)
      : TypeDefn(rib, span, name, runes, type) {}

public:
    [[nodiscard]]
    static AliasDefn* create(Rib& rib, SourceSpan span, 
                             const std::string& name, const Runes& runes, 
                             Type* type);

    ~AliasDefn() = default;

    AliasDefn(const AliasDefn&) = delete;
    void operator=(const AliasDefn&) = delete;

    AliasDefn(AliasDefn&&) noexcept = delete;
    void operator=(AliasDefn&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { visitor.visit(*this); }

    const AliasType* type() const { return static_cast<const AliasType*>(m_type); }
    AliasType* type() { return static_cast<AliasType*>(m_type); }
};

/// Represents a structure type definition.
class StructDefn final : public TypeDefn {
public:
    using Fields = std::vector<FieldDefn*>;
    using Methods = std::vector<FunctionDefn*>;

private:
    Fields m_fields = {};
    Methods m_methods = {};

    StructDefn(Rib* rib, SourceSpan span, const std::string& name, 
               const Runes& runes, Type* type)
      : TypeDefn(rib, span, name, runes, type) {}
      
public:
    [[nodiscard]]
    static StructDefn* create(Rib& rib, SourceSpan span, 
                              const std::string& name, const Runes& runes, 
                              Type* type);

    ~StructDefn() override;

    StructDefn(const StructDefn&) = delete;
    void operator=(const StructDefn&) = delete;

    StructDefn(StructDefn&&) noexcept = delete;
    void operator=(StructDefn&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { visitor.visit(*this); }

    const StructType* type() const { return static_cast<const StructType*>(m_type); }
    StructType* type() { return static_cast<StructType*>(m_type); }

    /// Set the field list of this structore to |fields|.
    void set_fields(const Fields& fields) { m_fields = fields; }

    /// Returns the field list of this structure.
    const Fields& fields() const { return m_fields; }
    Fields& fields() { return m_fields; }

    /// Returns the |i|-th field in this structure.
    const FieldDefn* get_field(uint32_t i) const {
        assert(i < m_fields.size() && "index out of bounds!");
        return m_fields[i];
    }

    FieldDefn* get_field(uint32_t i) {
        assert(i < m_fields.size() && "index out of bounds!");
        return m_fields[i];
    }

    /// Returns the field in this structure with the given |name| if it exists,
    /// and null otherwise.
    const FieldDefn* get_field(const std::string& name) const {
        for (const FieldDefn* field : m_fields) {
            if (field->name() == name)
                return field;
        }

        return nullptr;
    }

    FieldDefn* get_field(const std::string& name) {
        return const_cast<FieldDefn*>(
            static_cast<const StructDefn*>(this)->get_field(name));
    }

    /// Test if this structure has a field with the given |name|.
    bool has_field(const std::string& name) const {
        return get_field(name) != nullptr;
    }

    /// Returns the number of fields in this structure.
    uint32_t num_fields() const { return m_fields.size(); }

    /// Test if this structure has any fields.
    bool has_fields() const { return !m_fields.empty(); }

    /// Test if this structure is empty i.e. contains no fields.
    [[nodiscard]] bool empty() const { return m_fields.empty(); }

    /// Set the method list of this structure to |methods|.
    void set_methods(const Methods& methods) { m_methods = methods; }

    /// Returns the method list of this structure.
    const Methods& methods() const { return m_methods; }
    Methods& methods() { return m_methods; }

    /// Returns the method of this structure with the given |name| if it 
    /// exists, and null otherwise.
    const FunctionDefn* get_method(const std::string& name) const {
        for (const FunctionDefn* method : m_methods) {
            if (method->name() == name)
                return method;
        }

        return nullptr;
    }

    FunctionDefn* get_method(const std::string& name) {
        return const_cast<FunctionDefn*>(
            static_cast<const StructDefn*>(this)->get_method(name));
    }

    /// Test if this structure has a method with the given |name|.
    bool has_method(const std::string& name) const {
        return get_method(name) != nullptr;
    }

    /// Returns the number of methods this structure has.
    uint32_t num_methods() const { return m_methods.size(); }
    
    // Test if this structure has any methods.
    bool has_methods() const { return !m_methods.empty(); }
};

/// Represents an enumeration type definition.
class EnumDefn final : public TypeDefn {
public:
    using Variants = std::vector<VariantDefn*>;

private:
    Variants m_variants = {};

    EnumDefn(Rib* rib, SourceSpan span, const std::string& name, 
             const Runes& runes, Type* type)
      : TypeDefn(rib, span, name, runes, type) {}

public:
    [[nodiscard]]
    static EnumDefn* create(Rib& rib, SourceSpan span, 
                            const std::string& name, const Runes& runes, 
                            Type* type);

    ~EnumDefn() override;

    EnumDefn(const EnumDefn&) = delete;
    void operator=(const EnumDefn&) = delete;

    EnumDefn(EnumDefn&&) noexcept = delete;
    void operator=(EnumDefn&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { visitor.visit(*this); }

    const EnumType* type() const { return static_cast<const EnumType*>(m_type); }
    EnumType* type() { return static_cast<EnumType*>(m_type); }

    /// Set the variant list of this enum to |variants|.
    void set_variants(const Variants& variants) { m_variants = variants; }

    /// Returns the variant list of this enum.
    const Variants& variants() const { return m_variants; }
    Variants& variants() { return m_variants; }

    /// Returns the |i|-th variant in this enum.
    const VariantDefn* get_variant(uint32_t i) const {
        assert(i < m_variants.size() && "index out of bounds!");
        return m_variants[i];
    }

    VariantDefn* get_variant(uint32_t i) {
        assert(i < m_variants.size() && "index out of bounds!");
        return m_variants[i];
    }

    /// Returns the number of variants in this enum.
    uint32_t num_variants() const { return m_variants.size(); }

    /// Test if this enum has any variants.
    bool has_variants() const { return !m_variants.empty(); }

    /// Test if this enum is empty i.e. contains no variants.
    [[nodiscard]] bool empty() const { return m_variants.empty(); }
};

} // namespace lace

#endif // LACE_DEFN_H_
