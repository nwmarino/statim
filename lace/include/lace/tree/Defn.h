//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LOVELACE_DEFN_H_
#define LOVELACE_DEFN_H_

//
//  This header file declares a set of polymorphic classes for representing 
//  language definitions in the abstract syntax tree.
//

#include "lace/core/Common.h"
#include "lace/tree/AST.h"
#include "lace/tree/Rune.h"
#include "lace/tree/Type.h"
#include "lace/tree/VisitorBase.h"
#include "lace/types/SourceSpan.h"

#include <cassert>
#include <string>
#include <vector>

namespace lace {

class BlockStmt;
class Expr;
class Rune;
class Scope;
class Stmt;
class Type;

/// Base class for all definition types in the abstract syntax tree.
class Defn {
public:
    /// The different kinds of definitions.
    enum Kind : uint32_t {
        Alias,
        Enum,
        Field,
        Function,
        Load,
        Parameter,
        Struct,
        Variable,
        Variant,
    };

protected:
    /// The kind of definition this is.
    const Kind m_kind;

    /// The span of source code that this definition covers.
    const SourceSpan m_span;

    Defn(Kind kind, SourceSpan span) : m_kind(kind), m_span(span) {}

public:
    virtual ~Defn() = default;

    Defn(const Defn&) = delete;
    void operator=(const Defn&) = delete;

    Defn(Defn&&) noexcept = delete;
    void operator=(Defn&&) noexcept = delete;

    virtual void accept(VisitorBase& visitor) = 0;

    Kind get_kind() const { return m_kind; }

    /// Test if this is an alias definition.
    bool is_alias() const { return m_kind == Alias; }

    /// Test if this is an enum definition.
    bool is_enum() const { return m_kind == Enum; }

    /// Test if this is a field definition.
    bool is_field() const { return m_kind == Field; }

    /// Test if this is a function definition.
    bool is_function() const { return m_kind == Function; }

    /// Test if this is a load definition.
    bool is_load() const { return m_kind == Load; }

    /// Test if this is a parameter definition.
    bool is_parameter() const { return m_kind == Parameter; }

    /// Test if this is a struct definition.
    bool is_struct() const { return m_kind == Struct; }

    /// Test if this is a variable definition.
    bool is_variable() const { return m_kind == Variable; }

    /// Test if this is a variant definition.
    bool is_variant() const { return m_kind == Variant; }

    SourceSpan get_span() const { return m_span; }
};

/// Represents a top-level load definition.
class LoadDefn : public Defn {
    std::string m_path;

    LoadDefn(SourceSpan span, const std::string& path) 
      : Defn(Defn::Load, span), m_path(path) {}

public:
    [[nodiscard]]
    static LoadDefn* create(AST::Context& ctx, SourceSpan span, 
                            const std::string& path);

    ~LoadDefn() = default;

    LoadDefn(const LoadDefn&) = delete;
    void operator=(const LoadDefn&) = delete;

    LoadDefn(LoadDefn&&) noexcept = delete;
    void operator=(LoadDefn&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { visitor.visit(*this); }

    void set_path(const std::string& path) { m_path = path; }
    const std::string& get_path() const { return m_path; }
    std::string& get_path() { return m_path; }
};

/// Base class for definitions with a name and potential rune set.
class NamedDefn : public Defn {
protected:
    std::string m_name;
    std::vector<Rune*> m_runes;

    NamedDefn(Defn::Kind kind, SourceSpan span, const std::string& name, 
              const std::vector<Rune*>& runes)
      : Defn(kind, span), m_name(name), m_runes(runes) {}

public:
    virtual ~NamedDefn() override;

    NamedDefn(const NamedDefn&) = delete;
    void operator=(const NamedDefn&) = delete;

    NamedDefn(NamedDefn&&) noexcept = delete;
    void operator=(NamedDefn&&) noexcept = delete;

    void set_name(const std::string& name) { m_name = name; }
    const std::string& get_name() const { return m_name; }
    std::string& get_name() { return m_name; }

    const std::vector<Rune*>& get_runes() const { return m_runes; }
    std::vector<Rune*>& get_runes() { return m_runes; }

    void addRune(Rune* rune) {
        if (!hasRune(rune->getType()))
            m_runes.push_back(rune);
    }

    /// Returns the rune with the given |type| if this definition has one, and null otherwise.
    const Rune* getRune(Rune::Type type) const {
        for (Rune* rune : m_runes) {
            if (rune->getType() == type)
                return rune;
        }

        return nullptr;
    }

    Rune* get_rune(Rune::Type type) {
        return const_cast<Rune*>(static_cast<const NamedDefn*>(this)->get_rune(type));
    }

    const Rune* get_rune(uint32_t i) const {
        assert(i < num_runes() && "index out of bounds!");
        return m_runes[i];
    }

    Rune* get_rune(uint32_t i) {
        assert(i < num_runes() && "index out of bounds!");
        return m_runes[i];
    }
    
    /// Test if this definition has a rune of the given |type|.
    Result hasRune(Rune::Type type) const {
        for (Rune* rune : m_runes) {
            if (rune->hasType(type))
                return true;
        }
            
        return false;
    }

    uint32_t num_runes() const { return m_runes.size(); }
    bool has_runes() const { return !m_runes.empty(); }
};

/// Base class for all named definitions that are typed and produce a value.
class ValueDefn : public NamedDefn {
protected:
    QualType m_type;

    ValueDefn(Defn::Kind kind, SourceSpan span, const std::string& name, 
              const std::vector<Rune*>& runes, const QualType& type)
      : NamedDefn(kind, span, name, runes), m_type(type) {}

public:
    virtual ~ValueDefn() = default;

    ValueDefn(const ValueDefn&) = delete;
    void operator=(const ValueDefn&) = delete;

    ValueDefn(ValueDefn&&) noexcept = delete;
    void operator=(ValueDefn&&) noexcept = delete;

    void set_type(const QualType& type) { m_type = type; }
    const QualType& get_type() const { return m_type; }
    QualType& get_type() { return m_type; }
};

/// Represents a variable definition, either local or global.
class VariableDefn final : public ValueDefn {
    friend class SemanticAnalysis;

    // The initializing expression of this variable, if there is one.
    Expr* m_init;

    // If this is a global variable.
    bool m_global;

    VariableDefn(SourceSpan span, const std::string& name, const std::vector<Rune*>& runes, 
                 const QualType& type, Expr* init, bool global)
      : ValueDefn(Defn::Variable, span, name, runes, type), m_init(init), 
        m_global(global) {}

public:
    [[nodiscard]]
    static VariableDefn* create(AST::Context& ctx, SourceSpan span, 
                                const std::string& name, const std::vector<Rune*>& runes, 
                                const QualType& type, Expr* init, bool global);

    ~VariableDefn() override;

    VariableDefn(const VariableDefn&) = delete;
    void operator=(const VariableDefn&) = delete;

    VariableDefn(VariableDefn&&) noexcept = delete;
    void operator=(VariableDefn&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { visitor.visit(*this); }

    const Expr* get_init() const { return m_init; }
    Expr* get_init() { return m_init; }

    bool has_init() const { return m_init != nullptr; }

    bool is_global() const { return m_global; }
};

/// Represents a function parameter definition.
class ParameterDefn final : public ValueDefn {
    ParameterDefn(SourceSpan span, const std::string& name, const std::vector<Rune*>& runes,
                  const QualType& type)
      : ValueDefn(Defn::Parameter, span, name, runes, type) {}
      
public:
    [[nodiscard]]
    static ParameterDefn* create(AST::Context& ctx, SourceSpan span, 
                                 const std::string& name, const std::vector<Rune*>& runes,
                                 const QualType& type);

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

    /// The list of parameters for this function.
    Params m_params;

    /// The body of the function, if it has one.
    BlockStmt* m_body;

    FunctionDefn(SourceSpan span, const std::string& name, const std::vector<Rune*>& runes, 
                 const QualType& type, Scope* scope, const Params& params, 
                 BlockStmt* body)
      : ValueDefn(Defn::Function, span, name, runes, type), m_scope(scope), 
        m_params(params), m_body(body) {}

public:
    [[nodiscard]]
    static FunctionDefn* create(AST::Context& ctx, SourceSpan span, 
                                const std::string& name, const std::vector<Rune*>& runes, 
                                const QualType& type, Scope* scope, 
                                const Params& params, BlockStmt* body = nullptr);

    ~FunctionDefn() override;

    FunctionDefn(const FunctionDefn&) = delete;
    void operator=(const FunctionDefn&) = delete;

    FunctionDefn(FunctionDefn&&) noexcept = delete;
    void operator=(FunctionDefn&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { visitor.visit(*this); }

    /// Test if this is the main function, i.e. a function named `main`.
    bool is_main() const { return get_name() == "main"; }

    const QualType& get_return_type() const {
        return static_cast<const FunctionType*>(m_type.getType())->result();
    }

    const Scope* get_scope() const { return m_scope; }
    Scope* get_scope() { return m_scope; }

    void set_params(const Params& params) { m_params = params; }
    const Params& get_params() const { return m_params; }
    Params& get_params() { return m_params; }

    const ParameterDefn* get_param(uint32_t i) const {
        assert(i < num_params() && "index out of bounds!");
        return m_params[i];
    }

    ParameterDefn* get_param(uint32_t i) {
        assert(i < num_params() && "index out of bounds!");
        return m_params[i];
    }

    uint32_t num_params() const { return m_params.size(); }
    bool has_params() const { return !m_params.empty(); }

    void set_body(BlockStmt* body) { m_body = body; }
    const BlockStmt* get_body() const { return m_body; }
    BlockStmt* get_body() { return m_body; }

    bool has_body() const { return m_body != nullptr; }
};

/// Represents a field definition within a structure.
class FieldDefn final : public ValueDefn {
    uint32_t m_index;

    FieldDefn(SourceSpan span, const std::string& name, const std::vector<Rune*>& runes, 
              const QualType& type, uint32_t index)
      : ValueDefn(Defn::Field, span, name, runes, type), m_index(index) {}

public:
    [[nodiscard]]
    static FieldDefn* create(AST::Context& ctx, SourceSpan span, 
                             const std::string& name, const std::vector<Rune*>& runes, 
                             const QualType& type, uint32_t index);

    ~FieldDefn() = default;

    FieldDefn(const FieldDefn&) = delete;
    void operator=(const FieldDefn&) = delete;

    FieldDefn(FieldDefn&&) noexcept = delete;
    void operator=(FieldDefn&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { visitor.visit(*this); }

    uint32_t get_index() const { return m_index; }
};

/// Represents an enum variant definition.
class VariantDefn final : public ValueDefn {
    const int64_t m_value;

    VariantDefn(SourceSpan span, const std::string& name, const std::vector<Rune*>& runes, 
                const QualType& type, int64_t value)
      : ValueDefn(Defn::Variant, span, name, runes, type), m_value(value) {}

public:
    [[nodiscard]]
    static VariantDefn* create(AST::Context& ctx, SourceSpan span, 
                               const std::string& name, const std::vector<Rune*>& runes, 
                               const QualType& type, int64_t value);

    ~VariantDefn() = default;

    VariantDefn(const VariantDefn&) = delete;
    void operator=(const VariantDefn&) = delete;

    VariantDefn(VariantDefn&&) noexcept = delete;
    void operator=(VariantDefn&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { visitor.visit(*this); }
    
    int64_t get_value() const { return m_value; }
};

/// Base class for all named definitions that define a new type.
class TypeDefn : public NamedDefn {
protected:
    /// The type defined by this definition.
    const Type* m_type;

    TypeDefn(Defn::Kind kind, SourceSpan span, const std::string& name, 
             const std::vector<Rune*>& runes, const Type* type)
      : NamedDefn(kind, span, name, runes), m_type(type) {}

public:
    virtual ~TypeDefn() = default;

    TypeDefn(const TypeDefn&) = delete;
    void operator=(const TypeDefn&) = delete;

    TypeDefn(TypeDefn&&) noexcept = delete;
    void operator=(TypeDefn&&) noexcept = delete;

    void set_type(const Type* type) { m_type = type; }
    const Type* get_type() const { return m_type; }
};

/// Represents a type alias definition.
class AliasDefn final : public TypeDefn {
    AliasDefn(SourceSpan span, const std::string& name, const std::vector<Rune*>& runes, 
              const Type* type)
      : TypeDefn(Defn::Alias, span, name, runes, type) {}

public:
    [[nodiscard]]
    static AliasDefn* create(AST::Context& ctx, SourceSpan span, 
                             const std::string& name, const std::vector<Rune*>& runes, 
                             const Type* type);

    ~AliasDefn() = default;

    AliasDefn(const AliasDefn&) = delete;
    void operator=(const AliasDefn&) = delete;

    AliasDefn(AliasDefn&&) noexcept = delete;
    void operator=(AliasDefn&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { visitor.visit(*this); }
};

/// Represents a structure type definition.
class StructDefn final : public TypeDefn {
public:
    using Fields = std::vector<FieldDefn*>;
    
private:
    Fields m_fields = {};

    StructDefn(SourceSpan span, const std::string& name, const std::vector<Rune*>& runes, 
               const Type* type)
      : TypeDefn(Defn::Struct, span, name, runes, type) {}
      
public:
    [[nodiscard]]
    static StructDefn* create(AST::Context& ctx, SourceSpan span, 
                              const std::string& name, const std::vector<Rune*>& runes, 
                              const Type* type);

    ~StructDefn() override;

    StructDefn(const StructDefn&) = delete;
    void operator=(const StructDefn&) = delete;

    StructDefn(StructDefn&&) noexcept = delete;
    void operator=(StructDefn&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { visitor.visit(*this); }

    void set_fields(const Fields& fields) { m_fields = fields; }
    const Fields& get_fields() const { return m_fields; }
    Fields& get_fields() { return m_fields; }

    const FieldDefn* get_field(uint32_t i) const {
        assert(i < num_fields() && "index out of bounds!");
        return m_fields[i];
    }

    FieldDefn* get_field(uint32_t i) {
        assert(i < num_fields() && "index out of bounds!");
        return m_fields[i];
    }

    const FieldDefn* get_field(const std::string& name) const {
        for (const auto& field : m_fields)
            if (field->get_name() == name)
                return field;

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

    uint32_t num_fields() const { return m_fields.size(); }
    bool has_fields() const { return !m_fields.empty(); }
};

/// Represents an enumeration type definition.
class EnumDefn final : public TypeDefn {
public:
    using Variants = std::vector<VariantDefn*>;

private:
    Variants m_variants = {};

    EnumDefn(SourceSpan span, const std::string& name, const std::vector<Rune*>& runes, 
             const Type* type)
      : TypeDefn(Kind::Enum, span, name, runes, type) {}

public:
    [[nodiscard]]
    static EnumDefn* create(AST::Context& ctx, SourceSpan span, 
                            const std::string& name, const std::vector<Rune*>& runes, 
                            const Type* type);

    ~EnumDefn() override;

    EnumDefn(const EnumDefn&) = delete;
    void operator=(const EnumDefn&) = delete;

    EnumDefn(EnumDefn&&) noexcept = delete;
    void operator=(EnumDefn&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { visitor.visit(*this); }

    void set_variants(const Variants& variants) { m_variants = variants; }
    const Variants& get_variants() const { return m_variants; }
    Variants& get_variants() { return m_variants; }

    const VariantDefn* get_variant(uint32_t i) const {
        assert(i < num_variants() && "index out of bounds!");
        return m_variants[i];
    }

    VariantDefn* get_variant(uint32_t i) {
        assert(i < num_variants() && "index out of bounds!");
        return m_variants[i];
    }

    uint32_t num_variants() const { return m_variants.size(); }
    bool has_variants() const { return !m_variants.empty(); }
};

} // namespace lace

#endif // LOVELACE_DEFN_H_
