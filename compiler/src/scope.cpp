#include "ast.hpp"
#include "scope.hpp"

using namespace stm;

Scope::Scope(Props props, Scope* pParent) : props(props), pParent(pParent) {};

Decl* Scope::get(const std::string& name) {
    for (auto& [ sym_name, sym ] : symbols)
        if (sym_name == name)
            return sym;

    if (pParent)
        return pParent->get(name);

    return nullptr;
}

Result Scope::add(Decl* pDecl) {
    if (get(pDecl->get_name()) != nullptr)
        return Result::Duplicate;

    symbols.emplace(pDecl->get_name(), pDecl);
    return Result::Success;
}
