#include "tree/decl.hpp"
#include "tree/expr.hpp"
#include "tree/stmt.hpp"

using namespace stm;

Decl::Decl(const Span& span, const std::string& name, 
           const std::vector<Rune*>& decorators)
    : span(span), name(name), decorators(decorators) {}

bool Decl::has_decorator(Rune::Kind kind) const {
    for (auto& dec : decorators)
        if (dec->kind() == kind)
            return true;

    return false;
}

UseDecl::UseDecl(const Span& span, const std::string& path,
                 const std::vector<Rune*>& decorators)
    : Decl(span, path, decorators) {}

stm::FunctionDecl::FunctionDecl(
        const Span& span, 
        const std::string& name, 
        const std::vector<Rune*>& decorators, 
        const FunctionType* pType, 
        const std::vector<ParameterDecl*>& params,
        Scope* pScope,
        Stmt* pBody)
    : Decl(span, name, decorators), pType(pType), params(params), pScope(pScope), pBody(pBody) {}

stm::FunctionDecl::~FunctionDecl() {
    for (auto param : params) delete param;
    params.clear();
    
    delete pScope;
    pScope = nullptr;

    delete pBody;
    pBody = nullptr;
}

stm::ParameterDecl::ParameterDecl(
        const Span& span,
        const std::string& name,
        const std::vector<Rune*>& decorators,
        const Type* pType)
    : Decl(span, name, decorators), pType(pType) {}

VariableDecl::VariableDecl(const Span& span, const std::string& name,
                           const std::vector<Rune*>& decorators, const Type* ty,
                           Expr* init, bool global)
    : Decl(span, name, decorators), m_type(ty), m_init(init), 
      m_global(global) {}

stm::VariableDecl::~VariableDecl() {
    if (has_init())
        delete m_init;
}

stm::FieldDecl::FieldDecl(
        const Span& span,
        const std::string& name,
        const std::vector<Rune*>& runes,
        const Type* type,
        const StructDecl* parent,
        u32 index)
    : Decl(span, name, runes), m_type(type), m_parent(parent), m_index(index) {}

stm::StructDecl::StructDecl(
        const Span& span,
        const std::string& name,
        const std::vector<Rune*>& runes,
        const StructType* type,
        const std::vector<FieldDecl*>& fields)
    : Decl(span, name, runes), m_type(type), m_fields(fields) {
    for (auto field : fields) field->set_parent(this);
}

stm::StructDecl::~StructDecl() {
    for (auto field : m_fields) delete field;
    m_fields.clear();
}

bool stm::StructDecl::append_field(FieldDecl* field) {
    if (get_field(field->get_name())) 
        return false;
    
    field->set_parent(this);
    field->set_index(m_fields.size());
    m_fields.push_back(field);
    return true;
}

stm::EnumValueDecl::EnumValueDecl(
        const Span& span,
        const std::string& name,
        const std::vector<Rune*>& runes,
        const Type* type,
        i64 value)
    : Decl(span, name, runes), m_type(type), m_value(value) {}

stm::EnumDecl::EnumDecl(
        const Span& span,
        const std::string& name,
        const std::vector<Rune*>& runes,
        const EnumType* type,
        const std::vector<EnumValueDecl*>& values)
    : Decl(span, name, runes), m_type(type), m_values(values) {}

stm::EnumDecl::~EnumDecl() {
    for (auto value : m_values) delete value;
    m_values.clear();
}

bool stm::EnumDecl::append_value(EnumValueDecl* value) {
    if (get_value(value->get_name()))
        return false;

    m_values.push_back(value);
    return true;
}
