#ifndef STATIM_SCOPE_HPP_
#define STATIM_SCOPE_HPP_

#include "types.hpp"

#include <map>
#include <string>

namespace stm {
     
class Decl;

class Scope final {
public:
    enum class Context : u8 {
        Global, 
        Function, 
        Block,
    };

private:
    Scope*                          pParent;
    Context                         context;
    std::map<std::string, Decl*>    symbols;

public:
    Scope(Context context, Scope* pParent = nullptr);

    Scope* get_parent() const { return pParent; }

    Context get_props() const { return context; }

    const std::map<std::string, Decl*> &get_symbols() const { return symbols; }

    Decl* get(const std::string& name);

    Result add(Decl* pDecl);
};

} // namespace stm

#endif // STATIM_SCOPE_HPP_
