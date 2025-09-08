#ifndef STATIM_TREE_STMT_HPP_
#define STATIM_TREE_STMT_HPP_

#include "types/source_location.hpp"
#include "tree/visitor.hpp"

#include <vector>

namespace stm {

class Rune;

class Stmt {
    friend class SymbolAnalysis;
    friend class SemanticAnalysis;
    friend class Codegen;

protected:
    Span span;

public:
    Stmt(const Span& span) : span(span) {};

    virtual ~Stmt() = default;

    const Span& get_span() const { return span; }

    virtual void accept(Visitor& visitor) = 0;

    virtual void print(std::ostream& os) const = 0;
};

class BlockStmt final : public Stmt {
    friend class SymbolAnalysis;
    friend class SemanticAnalysis;
    friend class Codegen;

    std::vector<Rune*>  runes;
    std::vector<Stmt*>  stmts;
    Scope*              pScope;
    
public:
    BlockStmt(
        const Span& span, 
        const std::vector<Rune*>& runes, 
        const std::vector<Stmt*>& stmts, 
        Scope* pScope);

    ~BlockStmt() override;

    const std::vector<Rune*>& get_runes() const { return runes; }

    const std::vector<Stmt*>& get_stmts() const { return stmts; }

    u32 size() const { return stmts.size(); }

    const Scope* get_scope() const { return pScope; }

    bool is_empty() const { return stmts.empty(); }

    void accept(Visitor& visitor) override {
        visitor.visit(*this);
    }

    void print(std::ostream& os) const override;
};

class BreakStmt final : public Stmt {
    friend class SymbolAnalysis;
    friend class SemanticAnalysis;
    friend class Codegen;

public:
    BreakStmt(const Span& span) : Stmt(span) {};

    void accept(Visitor& visitor) override {
        visitor.visit(*this);
    }

    void print(std::ostream& os) const override;
};

class ContinueStmt final : public Stmt {
    friend class SymbolAnalysis;
    friend class SemanticAnalysis;
    friend class Codegen;

public:
    ContinueStmt(const Span& span) : Stmt(span) {};

    void accept(Visitor& visitor) override {
        visitor.visit(*this);
    }

    void print(std::ostream& os) const override;
};

class DeclStmt final : public Stmt {
    friend class SymbolAnalysis;
    friend class SemanticAnalysis;
    friend class Codegen;

    Decl* pDecl;

public:
    DeclStmt(const Span& span, Decl* pDecl);
    ~DeclStmt() override;

    const Decl* get_decl() const { return pDecl; }

    void accept(Visitor& visitor) override {
        visitor.visit(*this);
    }

    void print(std::ostream& os) const override;
};

class IfStmt final : public Stmt {
    friend class SymbolAnalysis;
    friend class SemanticAnalysis;
    friend class Codegen;

    Expr* pCond;
    Stmt* pThen;
    Stmt* pElse;

public:
    IfStmt(const Span& span, Expr* pCond, Stmt* pThen, Stmt* pElse);
    ~IfStmt() override;

    const Expr* get_cond() const { return pCond; }

    const Stmt* get_then() const { return pThen; }

    const Stmt* get_else() const { return pElse; }

    bool has_else() const { return pElse != nullptr; }

    void accept(Visitor& visitor) override {
        visitor.visit(*this);
    }

    void print(std::ostream& os) const override;
};

class WhileStmt final : public Stmt {
    friend class SymbolAnalysis;
    friend class SemanticAnalysis;
    friend class Codegen;

    Expr* pCond;
    Stmt* pBody;

public:
    WhileStmt(const Span& span, Expr* pCond, Stmt* pBody);
    ~WhileStmt() override;

    const Expr* get_cond() const { return pCond; }

    const Stmt* get_body() const { return pBody; }

    bool has_body() const { return pBody != nullptr; }

    void accept(Visitor& visitor) override {
        visitor.visit(*this);
    }

    void print(std::ostream& os) const override;
};

class RetStmt final : public Stmt {
    friend class SymbolAnalysis;
    friend class SemanticAnalysis;
    friend class Codegen;

    Expr* pExpr;

public:
    RetStmt(const Span& span, Expr* pExpr);
    ~RetStmt() override;

    const Expr* get_expr() const { return pExpr; }

    bool has_expr() const { return pExpr != nullptr; }

    void accept(Visitor& visitor) override {
        visitor.visit(*this);
    }

    void print(std::ostream& os) const override;
};

} // namespace stm

#endif // STATIM_TREE_STMT_HPP_
