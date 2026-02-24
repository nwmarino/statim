//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LACE_STMT_H_
#define LACE_STMT_H_

//
//  This header file declares a set of polymorphic classes for representing 
//  statements in the abstract syntax tree.
//

#include "lace/tree/AST.h"
#include "lace/tree/Rune.h"
#include "lace/tree/VisitorBase.h"
#include "lace/types/SourceSpan.h"

#include <cassert>

namespace lace {

class Defn;
class Expr;
class Scope;

/// Base class for all statement nodes in the abstract syntax tree.
class Stmt {
protected:
    /// The span of source code that this statement covers.
    const SourceSpan m_span;

    Stmt(SourceSpan span) : m_span(span) {}

public:
    virtual ~Stmt() = default;

    Stmt(const Stmt&) = delete;
    void operator=(const Stmt&) = delete;
    
    Stmt(Stmt&&) noexcept = delete;
    void operator=(Stmt&&) noexcept = delete;

    virtual void accept(VisitorBase& visitor) = 0;
    
    /// Returns the span of source code which this statement covers.
    SourceSpan get_span() const { return m_span; }
};

/// Represents a statement that adapts either a nested definition or expression.
class AdapterStmt final : public Stmt {
public:
    /// The different flavors of adaptiveness.
    enum class Kind : uint32_t {
        Definitive,
        Expressive,  
    };

private:
    Kind m_kind;
    union {
        Defn* m_defn;
        Expr* m_expr;
    };

    AdapterStmt(SourceSpan span, Defn* defn) 
      : Stmt(span), m_kind(Kind::Definitive), m_defn(defn) {}

    AdapterStmt(SourceSpan span, Expr* expr) 
      : Stmt(span), m_kind(Kind::Expressive), m_expr(expr) {}

public:
    [[nodiscard]]
    static AdapterStmt* create(AST& ast, Defn* defn);
    
    [[nodiscard]]
    static AdapterStmt* create(AST& ast, Expr* expr);

    ~AdapterStmt() override;

    AdapterStmt(const AdapterStmt&) = delete;
    void operator=(const AdapterStmt&) = delete;

    AdapterStmt(AdapterStmt&&) noexcept = delete;
    void operator=(AdapterStmt&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { 
        visitor.visit(*this); 
    }

    /// Returns the kind of adapter this is.
    Kind kind() const { return m_kind; }

    /// Test if this is a definitive adapter statement, i.e. nests a
    /// definition.
    bool is_definitive() const { return m_kind == Kind::Definitive; }

    /// Test if this is an expressive adapter statement, i.e. nests an
    /// expression.
    bool is_expressive() const { return m_kind == Kind::Expressive; }

    /// Returns the definition of this adapter statement.
    const Defn* defn() const { 
        assert(is_definitive() && "invalid adapter!");
        return m_defn;
    }

    Defn* defn() {
        assert(is_definitive() && "invalid adapter!");
        return m_defn;
    }

    /// Returns the expression of this adapter statement.
    const Expr* expr() const { 
        assert(is_expressive() && "invalid adapter!");
        return m_expr; 
    }

    Expr* expr() { 
        assert(is_expressive() && "invalid adapter!");
        return m_expr; 
    }
};

/// Represents a series of statements enclosed by curly braces `{, }`.
class BlockStmt final : public Stmt {
public:
    using Stmts = std::vector<Stmt*>;
    
private:
    Scope* m_scope;
    Stmts m_stmts;

    BlockStmt(SourceSpan span, Scope* scope, const Stmts& stmts)
      : Stmt(span), m_scope(scope), m_stmts(stmts) {}

public:
    [[nodiscard]]
    static BlockStmt* create(AST& ast, SourceSpan span, Scope* scope,
                             const Stmts& stmts);

    ~BlockStmt() override;

    BlockStmt(const BlockStmt&) = delete;
    void operator=(const BlockStmt&) = delete;

    BlockStmt(BlockStmt&&) noexcept = delete;
    void operator=(BlockStmt&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { visitor.visit(*this); }

    /// Returns the scope tree of this statement.
    const Scope* scope() const { return m_scope; }
    Scope* scope() { return m_scope; }

    /// Set the statement list of this block to |stmts|.
    void set_stmts(const Stmts& stmts) { m_stmts = stmts; }

    /// Returns the statement list of this block.
    const Stmts& stmts() const { return m_stmts; }
    Stmts& stmts() { return m_stmts; }

    /// Returns the |i|-th statement in this block.
    const Stmt* get_stmt(uint32_t i) const {
        assert(i < m_stmts.size() && "index out of bounds!");
        return m_stmts[i];
    }

    Stmt* get_stmt(uint32_t i) {
        assert(i < m_stmts.size() && "index out of bounds!");
        return m_stmts[i];
    }

    /// Returns the number of statement in this block.
    uint32_t num_stmts() const { return m_stmts.size(); }

    /// Test if this block has any statements.
    bool has_stmts() const { return !m_stmts.empty(); }

    /// Test if this block is empty i.e. contains no statements.
    [[nodiscard]] bool empty() const { return m_stmts.empty(); }
};

/// Represents an 'if' statement.
class IfStmt final : public Stmt {
    Expr* m_cond;
    Stmt* m_then;
    Stmt* m_else;

    IfStmt(SourceSpan span, Expr* cond, Stmt* then, Stmt* els)
      : Stmt(span), m_cond(cond), m_then(then), m_else(els) {}

public:
    [[nodiscard]]
    static IfStmt* create(AST& ast, SourceSpan span, Expr* cond, 
                          Stmt* then, Stmt* els);

    ~IfStmt() override;

    IfStmt(const IfStmt&) = delete;
    void operator=(const IfStmt&) = delete;

    IfStmt(IfStmt&&) noexcept = delete;
    void operator=(IfStmt&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { visitor.visit(*this); }

    /// Returns the condition expression of this statement.
    const Expr* condition() const { return m_cond; }
    Expr* condition() { return m_cond; }

    /// Returns the 'then' clause of this statement.
    const Stmt* then_body() const { return m_then; }
    Stmt* then_body() { return m_then; }

    /// Returns the 'else' clause of this statement, if it has one, and null
    /// otherwise.
    const Stmt* else_body() const { return m_else; }
    Stmt* else_body() { return m_else; }

    /// Test if this statement includes an 'else' clause.
    bool has_else() const { return m_else != nullptr; }
};

/// Represents a 'restart' statement.
class RestartStmt final : public Stmt {
    RestartStmt(SourceSpan span) : Stmt(span) {}

public:
    [[nodiscard]]
    static RestartStmt* create(AST& ast, SourceSpan span);

    ~RestartStmt() = default;

    RestartStmt(const RestartStmt&) = delete;
    void operator=(const RestartStmt&) = delete;

    RestartStmt(RestartStmt&&) noexcept = delete;
    void operator=(RestartStmt&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { visitor.visit(*this); }
};

/// Represents a 'ret' statement.
class RetStmt final : public Stmt {
    friend class SemanticAnalysis;

    /// The return expression, if there is one.
    Expr* m_expr;

    RetStmt(SourceSpan span, Expr* expr) : Stmt(span), m_expr(expr) {}

public:
    [[nodiscard]]
    static RetStmt* create(AST& ast, SourceSpan span, Expr* expr);

    ~RetStmt() override;

    RetStmt(const RetStmt&) = delete;
    void operator=(const RetStmt&) = delete;

    RetStmt(RetStmt&&) noexcept = delete;
    void operator=(RetStmt&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { visitor.visit(*this); }

    /// Returns the expression which this statement returns with.
    const Expr* expr() const { return m_expr; }
    Expr* expr() { return m_expr; }

    /// Test if this statement contains an expression.
    bool has_expr() const { return m_expr != nullptr; }
};

/// Represents a 'stop' statement.
class StopStmt final : public Stmt {
    StopStmt(SourceSpan span) : Stmt(span) {}

public:
    [[nodiscard]]
    static StopStmt* create(AST& ast, SourceSpan span);
    
    ~StopStmt() = default;

    StopStmt(const StopStmt&) = delete;
    void operator=(const StopStmt&) = delete;

    StopStmt(StopStmt&&) noexcept = delete;
    void operator=(StopStmt&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { visitor.visit(*this); }
};

/// Represents a 'until' statement.
class UntilStmt final : public Stmt {
    Expr* m_cond;
    Stmt* m_body;

    UntilStmt(SourceSpan span, Expr* cond, Stmt* body)
      : Stmt(span), m_cond(cond), m_body(body) {}

public:
    [[nodiscard]]
    static UntilStmt* create(AST& ast, SourceSpan span, Expr* cond, 
                             Stmt* body);

    ~UntilStmt() override;

    UntilStmt(const UntilStmt&) = delete;
    void operator=(const UntilStmt&) = delete;

    UntilStmt(UntilStmt&&) noexcept = delete;
    void operator=(UntilStmt&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { visitor.visit(*this); }

    /// Returns the condition expression of this statement.
    const Expr* condition() const { return m_cond; }
    Expr* condition() { return m_cond; }

    /// Returns the body of this statement if it has one, and null otherwise.
    const Stmt* body() const { return m_body; }
    Stmt* body() { return m_body; }

    /// Test if this statement has a body.
    bool has_body() const { return m_body != nullptr; }
};

/// Represents a statement which encapsulates a rune.
class RuneStmt final : public Stmt {
    Rune* m_rune;

    RuneStmt(SourceSpan span, Rune* rune) : Stmt(span), m_rune(rune) {}

public:
    [[nodiscard]] 
    static RuneStmt* create(AST& ast, SourceSpan span, Rune* rune);

    ~RuneStmt() override;

    RuneStmt(const RuneStmt&) = delete;
    void operator=(const RuneStmt&) = delete;

    RuneStmt(RuneStmt&&) noexcept = delete;
    void operator=(RuneStmt&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { visitor.visit(*this); }

    /// Returns the rune which this statement encapsulates.
    const Rune* rune() const { return m_rune; }
    Rune* rune() { return m_rune; }
};

} // namespace lace

#endif // LACE_STMT_H_
