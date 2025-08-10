#ifndef STATIM_SCOPE_HPP_
#define STATIM_SCOPE_HPP_

#include "types.hpp"

#include <map>
#include <string>

namespace stm {
     
class Decl;

class Scope final {
public:
    enum class Props : u8 {
        Global, 
        Function, 
        Block,
    };

private:
    Scope*                          pParent;
    Props                           props;
    std::map<std::string, Decl*>    symbols;

public:
    Scope(Props props, Scope* pParent = nullptr);

    Scope* get_parent() const { return pParent; }

    Props get_props() const { return props; }

    const std::map<std::string, Decl*> &get_symbols() const { return symbols; }

    Decl* get(const std::string& name);

    Result add(Decl* pDecl);
};

} // namespace stm

#endif // STATIM_SCOPE_HPP_
