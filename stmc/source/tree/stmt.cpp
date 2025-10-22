#include "tree/decl.hpp"
#include "tree/rune.hpp"
#include "tree/stmt.hpp"

using namespace stm;

AsmStmt::AsmStmt(const Span& span, const std::string& str,
                 const std::vector<std::string>& inputs,
                 const std::vector<std::string>& outputs,
                 const std::vector<Expr*>& exprs,
                 const std::vector<std::string>& clobbers,
                 bool is_volatile)
    : Stmt(span), m_asm(str), m_inputs(inputs), m_outputs(outputs), 
      m_exprs(exprs), m_clobbers(clobbers), m_volatile(is_volatile) {}

AsmStmt::~AsmStmt() {
    for (auto& expr : m_exprs) 
        delete expr;
        
    m_exprs.clear();
}

stm::BlockStmt::BlockStmt(
        const Span& span, 
        const std::vector<Rune*>& runes, 
        const std::vector<Stmt*>& stmts, 
        Scope* pScope)
    : Stmt(span), runes(runes), stmts(stmts), pScope(pScope) {};

stm::BlockStmt::~BlockStmt() {
    for (Rune* rune : runes) delete rune;
    runes.clear();
    
    for (Stmt* stmt : stmts) delete stmt;
    stmts.clear();

    delete pScope;
    pScope = nullptr;
}

stm::DeclStmt::DeclStmt(const Span& span, Decl* pDecl) : Stmt(span), pDecl(pDecl) {}

stm::DeclStmt::~DeclStmt() {
    delete pDecl;
    pDecl = nullptr;
}

stm::IfStmt::IfStmt(const Span& span, Expr* pCond, Stmt* pThen, Stmt* pElse)
    : Stmt(span), pCond(pCond), pThen(pThen), pElse(pElse) {}

stm::IfStmt::~IfStmt() {
    if (pCond != nullptr) {
        delete pCond;
        pCond = nullptr;
    }

    if (pThen != nullptr) {
        delete pThen;
        pThen = nullptr;
    }

    if (pElse != nullptr) {
        delete pElse;
        pElse = nullptr;
    }
}

stm::WhileStmt::WhileStmt(const Span& span, Expr* pCond, Stmt* pBody) 
    : Stmt(span), pCond(pCond), pBody(pBody) {}

stm::WhileStmt::~WhileStmt() {
    if (pCond != nullptr) {
        delete pCond;
        pCond = nullptr;
    }

    if (pBody != nullptr) {
        delete pBody;
        pBody = nullptr;
    }
}

stm::RetStmt::RetStmt(const Span& span, Expr* pExpr) 
    : Stmt(span), pExpr(pExpr) {}

stm::RetStmt::~RetStmt() {
    if (pExpr != nullptr) {
        delete pExpr;
        pExpr = nullptr;
    }
}
