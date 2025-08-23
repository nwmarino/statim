#ifndef STATIM_TREE_DECL_HPP_
#define STATIM_TREE_DECL_HPP_

#include "core/type.hpp"
#include "tree/visitor.hpp"
#include "types/source_location.hpp"

#include <string>
#include <vector>

namespace stm {

class Rune;

class Decl {
    friend class SymbolAnalysis;
    friend class SemanticAnalysis;
    friend class Codegen;
    
protected:
    Span                span;
    std::string         name;
    std::vector<Rune*>  decorators;

public:
    Decl(
        const Span& span, 
        const std::string& name, 
        const std::vector<Rune*>& decorators);

    virtual ~Decl() = default;

    virtual void accept(Visitor& visitor) = 0;

    virtual void print(std::ostream& os) const = 0;

    /// \returns The span of source code this declaration covers.
    Span& get_span() { return span; }

    /// \returns The span of source code this declaration covers.
    const Span& get_span() const { return span; }

    /// \returns The name of this declaration, if it is named.
    const std::string& get_name() const { return name; }

    const std::vector<Rune*>& get_decorators() const { return decorators; }
};

class FunctionDecl final : public Decl {
    friend class SymbolAnalysis;
    friend class SemanticAnalysis;
    friend class Codegen;

    const FunctionType*         pType;
    std::vector<ParameterDecl*> params; 
    Scope*                      pScope;
    Stmt*                       pBody;

public:
    FunctionDecl(
        const Span& span, 
        const std::string& name, 
        const std::vector<Rune*>& decorators, 
        const FunctionType* pType, 
        const std::vector<ParameterDecl*>& params,
        Scope* pScope,
        Stmt* pBody);

    ~FunctionDecl() override;

    const FunctionType* get_type() const { return pType; }

    const std::vector<ParameterDecl*>& get_params() const { return params; }

    const ParameterDecl* get_param(u32 idx) const { return params.at(idx); }

    u32 num_params() const { return params.size(); }

    const Scope* get_scope() const { return pScope; }

    const Stmt* get_body() const { return pBody; }

    bool returns() const { return pType->get_return_type()->is_void(); }

    bool has_params() const { return params.empty(); }

    bool has_body() const { return pBody != nullptr; }

    void accept(Visitor& visitor) override {
        visitor.visit(*this);
    }

    void print(std::ostream& os) const override;
};

class ParameterDecl final : public Decl {
    friend class SymbolAnalysis;
    friend class SemanticAnalysis;
    friend class Codegen;

    const Type* pType;

public:
    ParameterDecl(
        const Span& span,
        const std::string& name,
        const std::vector<Rune*>& decorators,
        const Type* pType);

    const Type* get_type() const { return pType; }

    void accept(Visitor& visitor) override {
        visitor.visit(*this);
    }

    void print(std::ostream& os) const override;
};

class VariableDecl final : public Decl {
    friend class SymbolAnalysis;
    friend class SemanticAnalysis;
    friend class Codegen;

    const Type* pType;
    Expr*       pInit;

public:
    VariableDecl(
        const Span& span,
        const std::string& name,
        const std::vector<Rune*>& decorators,
        const Type* pType,
        Expr* pInit);

    ~VariableDecl() override;

    const Type* get_type() const { return pType; }

    const Expr* get_init() const { return pInit; }

    bool has_init() const { return pInit != nullptr; }

    void accept(Visitor& visitor) override {
        visitor.visit(*this);
    }

    void print(std::ostream& os) const override;
};

/// Represents the declaration of a field within a structure.
class FieldDecl final : public Decl {
    friend class SymbolAnalysis;
    friend class SemanticAnalysis;
    friend class Codegen;

    const Type* m_type;
    const StructDecl* m_parent;
    u32 m_index;

public:
    FieldDecl(
        const Span& span,
        const std::string& name,
        const std::vector<Rune*>& runes,
        const Type* type,
        const StructDecl* parent,
        u32 index);

    /// \returns The type of this field.
    const Type* get_type() const { return m_type; }

    /// \returns The parent structure of this field.
    const StructDecl* get_parent() const { return m_parent; }

    /// Set the parent of this field to \p parent.
    void set_parent(const StructDecl* parent) { m_parent = parent; }

    /// \returns The index of this field in its parent structure.
    u32 get_index() const { return m_index; }

    /// Set the index of this field to \p index.
    void set_index(u32 index) { m_index = index; }

    void accept(Visitor& visitor) override {
        visitor.visit(*this);
    }

    void print(std::ostream& os) const override;
};

/// Represents the declaration of a structure.
class StructDecl final : public Decl {
    friend class SymbolAnalysis;
    friend class SemanticAnalysis;
    friend class Codegen;

    const StructType* m_type;
    std::vector<FieldDecl*> m_fields;

public:
    StructDecl(
        const Span& span,
        const std::string& name,
        const std::vector<Rune*>& runes,
        const StructType* type,
        const std::vector<FieldDecl*>& fields);

    ~StructDecl() override;

    /// \returns The type of this structure.
    const StructType* get_type() const { return m_type; }

    /// Set the type of this structure to \p type.
    void set_type(const StructType* type) { m_type = type; }

    /// \returns The fields of this structure.
    const std::vector<FieldDecl*>& get_fields() const { return m_fields; }

    /// \returns A field of this structure by name, if it exists.
    FieldDecl* get_field(const std::string& name) {
        for (auto field : m_fields)
            if (field->get_name() == name) return field;
    
        return nullptr;
    }

    /// \returns A field of this structure by name, if it exists.
    const FieldDecl* get_field(const std::string& name) const {
        for (auto field : m_fields)
            if (field->get_name() == name) return field;
    
        return nullptr;
    }

    /// \returns The number of fields in this structure.
    u32 num_fields() const { return m_fields.size(); }

    /// Append \p field to this structure.
    /// \returns `false` if the field has naming conflicts.
    bool append_field(FieldDecl* field);

    void accept(Visitor& visitor) override {
        visitor.visit(*this);
    }

    void print(std::ostream& os) const override;
};

/// Represents a valued variant of an enumeration. 
class EnumValueDecl final : public Decl {
    friend class SymbolAnalysis;
    friend class SemanticAnalysis;
    friend class Codegen;

    const EnumType* m_type;
    i64 m_value;

public:
    EnumValueDecl(
        const Span& span,
        const std::string& name,
        const std::vector<Rune*>& runes,
        const EnumType* type,
        i64 value);

    /// \returns The type of this enum variant.
    const EnumType* get_type() const { return m_type; }

    /// \returns The value of this enum variant.
    i64 get_value() const { return m_value; }

    void accept(Visitor& visitor) override {
        visitor.visit(*this);
    }

    void print(std::ostream& os) const override;
};

/// Represents the declaration of an enum and its variants.
class EnumDecl final : public Decl {
    friend class SymbolAnalysis;
    friend class SemanticAnalysis;
    friend class Codegen;

    const EnumType* m_type;
    std::vector<EnumValueDecl*> m_values;

public:
    EnumDecl(
        const Span& span,
        const std::string& name,
        const std::vector<Rune*>& runes,
        const EnumType* type,
        const std::vector<EnumValueDecl*>& values);

    ~EnumDecl() override;

    /// \returns The type defined by this enum declaration.
    const EnumType* get_type() const { return m_type; }

    /// Set the type of this enum to \p type.
    void set_type(const EnumType* type) { m_type = type; }

    /// \returns All the variants of this enum.
    const std::vector<EnumValueDecl*>& get_values() const { return m_values; }

    /// \returns A variant of this enum by name, if it exists.
    EnumValueDecl* get_value(const std::string& name) {
        for (auto value : m_values)
            if (value->get_name() == name) return value;
    
        return nullptr;
    }

    /// \returns A variant of this enum by name, if it exists.
    const EnumValueDecl* get_value(const std::string& name) const {
        for (auto value : m_values)
            if (value->get_name() == name) return value;
    
        return nullptr;
    }

    /// \returns The number of values in this enum.
    u32 num_values() const { return m_values.size(); }

    /// Append \p value to this enum.
    /// \returns `false` if the value has naming conflicts.
    bool append_value(EnumValueDecl* value);

    void accept(Visitor& visitor) override {
        visitor.visit(*this);
    }

    void print(std::ostream& os) const override;
};

} // namespace stm

#endif // STATIM_TREE_DECL_HPP_
