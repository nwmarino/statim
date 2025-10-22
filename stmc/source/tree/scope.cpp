#include "tree/decl.hpp"
#include "tree/scope.hpp"

stm::Decl* stm::Scope::get(const std::string& name) const {
    for (auto [ sym_name, sym ] : m_symbols)
        if (sym_name == name) return sym;

    if (m_parent != nullptr)
        return m_parent->get(name);

    return nullptr;
}

bool stm::Scope::add(Decl* decl) {
    assert(decl);

    if (get(decl->get_name()) != nullptr)
        return false;

    m_symbols.emplace(decl->get_name(), decl);
    return true;
}
