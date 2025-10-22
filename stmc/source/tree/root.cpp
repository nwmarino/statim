#include "core/logger.hpp"
#include "tree/decl.hpp"
#include "tree/root.hpp"

using namespace stm;

const stm::Type* stm::TypeContext::get(const std::string& name) const {
    auto it = types.find(name);
    if (it != types.end())
        return it->second;

    return nullptr;
}

const stm::BuiltinType* stm::TypeContext::get(BuiltinType::Kind kind) const {
    return builtins.at(kind);
}

const stm::DeferredType* stm::TypeContext::get(
        const DeferredType::Context& context) {
    DeferredType* type = new DeferredType(context);
    deferred.push_back(type);
    return type;
}

const stm::FunctionType* stm::TypeContext::get(
        const Type* pReturn, const std::vector<const Type*> &params) {
    FunctionType* type = new FunctionType(pReturn, params);
    functions.push_back(type);
    return type;
}

const stm::PointerType* stm::TypeContext::get(const Type* pPointee) {
    auto it = pointers.find(pPointee);
    if (it != pointers.end())
        return it->second;

    PointerType* type = new PointerType(pPointee);
    pointers.emplace(pPointee, type);
    return type;
}

const stm::StructType* stm::TypeContext::create(
        const std::vector<const Type*>& fields, const StructDecl* decl) {
    StructType* type = new StructType(fields, decl);
    types.emplace(decl->get_name(), type);
    structs.push_back(type);
    return type;
}

const stm::EnumType* stm::TypeContext::create(const Type* underlying, const EnumDecl* decl) {
    EnumType* type = new EnumType(underlying, decl);
    types.emplace(decl->get_name(), type);
    enums.push_back(type);
    return type;
}

stm::TypeContext::TypeContext() {
    for (auto kind = BuiltinType::Kind::Void; 
          kind <= BuiltinType::Kind::Float64; 
          kind = BuiltinType::Kind(u8(kind) + 1)) {
        BuiltinType* type = new BuiltinType(kind);
        builtins.emplace(kind, type);
        types.emplace(BuiltinType::get_name(kind), type);
    }
}

stm::TypeContext::~TypeContext() {
    for (auto [kind, type] : builtins)
        delete type;

    for (auto& type : deferred)
        delete type;

    for (auto& type : functions)
        delete type;

    for (auto [pointee, type] : pointers)
        delete type;

    builtins.clear();
    deferred.clear();
    functions.clear();
    pointers.clear();
}

Root::Root(InputFile& file, Scope* scope) : m_file(file), m_scope(scope) {}

stm::Root::~Root() {
    delete m_scope;

    for (auto& decl : decls()) 
        delete decl;

    m_decls.clear();
    m_imports.clear();
    m_exports.clear();
}

std::vector<UseDecl*> Root::uses() const {
    std::vector<UseDecl*> uses = {};
    for (auto& decl : decls())
        if (auto* use = dynamic_cast<UseDecl*>(decl))
            uses.push_back(use);

    return uses;
}

void stm::Root::validate() {
    // For each type which was deferred at parse-time, we need to resolve it
    // based on the context in which it was parsed.
    for (auto& deferred : m_context.deferred) {
        const DeferredType::Context& ctx = deferred->get_context();

        if (ctx.meta.line == 201 && ctx.base == "u64") {
            int x = 1;
        }

        // Try to resolve the base of the type.
        const Type* type = m_context.get(ctx.base);
        if (!type)
            stm::Logger::fatal("unresolved type: " + ctx.base);

        // Add however much indirection is needed for the type.
        for (u32 idx = 0; idx != ctx.indirection; ++idx)
            type = PointerType::get(*this, type);

        if (type->is_mut())
            type = type->as_mut();

        deferred->set_resolved(type);
    }
}
