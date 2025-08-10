#ifndef STATIM_AST_HPP_
#define STATIM_AST_HPP_

#include <string>
namespace stm {

class Root final {

};

class Decl {


public:
    std::string get_name() const;
};

class FunctionDecl final : public Decl {

};

class Stmt {

};

class BlockStmt final : public Stmt {

};

class RetStmt final : public Stmt {

};

class Expr : public Stmt {

};

class IntegerExpr final : public Expr {

};

} // namespace stm

#endif // STATIM_AST_HPP_
