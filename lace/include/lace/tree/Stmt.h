//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#ifndef LOVELACE_STMT_H_
#define LOVELACE_STMT_H_

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
public:
    /// The different kinds of statements.
    enum class Kind : uint32_t {
        Adapter,
        Block,
        If,
        Restart,
        Ret,
        Rune,
        Stop,
        Until,
    };

protected:
    /// The kind of statement this is.
    const Kind m_kind;

    /// The span of source code that this statement covers.
    const SourceSpan m_span;

    Stmt(Kind kind, SourceSpan span) : m_kind(kind), m_span(span) {}

public:
    virtual ~Stmt() = default;

    Stmt(const Stmt&) = delete;
    void operator=(const Stmt&) = delete;
    
    Stmt(Stmt&&) noexcept = delete;
    void operator=(Stmt&&) noexcept = delete;

    virtual void accept(VisitorBase& visitor) = 0;

    Kind get_kind() const { return m_kind; }
    
    SourceSpan get_span() const { return m_span; }
};

/// Represents a statement that adapts either a nested definition or expression.
class AdapterStmt final : public Stmt {
public:
    /// The different flavors of adaptiveness.
    enum Flavor : uint32_t {
        Definitive,
        Expressive,  
    };

private:
    Flavor m_flavor;
    union {
        Defn* m_defn;
        Expr* m_expr;
    };

    AdapterStmt(SourceSpan span, Defn* defn) 
      : Stmt(Stmt::Kind::Adapter, span), m_flavor(Definitive), m_defn(defn) {}

    AdapterStmt(SourceSpan span, Expr* expr) 
      : Stmt(Stmt::Kind::Adapter, span), m_flavor(Expressive), m_expr(expr) {}

public:
    [[nodiscard]]
    static AdapterStmt* create(AST::Context& ctx, Defn* defn);
    
    [[nodiscard]]
    static AdapterStmt* create(AST::Context& ctx, Expr* expr);

    ~AdapterStmt() override;

    AdapterStmt(const AdapterStmt&) = delete;
    void operator=(const AdapterStmt&) = delete;

    AdapterStmt(AdapterStmt&&) noexcept = delete;
    void operator=(AdapterStmt&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { 
        visitor.visit(*this); 
    }

    Flavor get_flavor() const { return m_flavor; }

    /// Test if this is a definitive adapter statement, i.e. nests a
    /// definition.
    bool is_definitive() const { return m_flavor == Definitive; }

    /// Test if this is an expressive adapter statement, i.e. nests an
    /// expression.
    bool is_expressive() const { return m_flavor == Expressive; }

    const Defn* get_defn() const { return m_defn; }
    Defn* get_defn() { return m_defn; }

    const Expr* get_expr() const { return m_expr; }
    Expr* get_expr() { return m_expr; }
};

/// Represents a series of statements enclosed by curly braces `{, }`.
class BlockStmt final : public Stmt {
public:
    using Stmts = std::vector<Stmt*>;
    
private:
    Scope* m_scope;
    Stmts m_stmts;

    BlockStmt(SourceSpan span, Scope* scope, const Stmts& stmts)
      : Stmt(Stmt::Kind::Block, span), m_scope(scope), m_stmts(stmts) {}

public:
    [[nodiscard]]
    static BlockStmt* create(AST::Context& ctx, SourceSpan span, Scope* scope,
                             const Stmts& stmts);

    ~BlockStmt() override;

    BlockStmt(const BlockStmt&) = delete;
    void operator=(const BlockStmt&) = delete;

    BlockStmt(BlockStmt&&) noexcept = delete;
    void operator=(BlockStmt&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { visitor.visit(*this); }

    const Scope* get_scope() const { return m_scope; }
    Scope* get_scope() { return m_scope; }

    void set_stmts(const Stmts& stmts) { m_stmts = stmts; }
    const Stmts& get_stmts() const { return m_stmts; }
    Stmts& get_stmts() { return m_stmts; }

    const Stmt* get_stmt(uint32_t i) const {
        assert(i < num_stmts() && "index out of bounds!");
        return m_stmts[i];
    }

    Stmt* get_stmt(uint32_t i) {
        assert(i < num_stmts() && "index out of bounds!");
        return m_stmts[i];
    }

    uint32_t num_stmts() const { return m_stmts.size(); }
    bool has_stmts() const { return !m_stmts.empty(); }
};

/// Represents an `if` statement.
class IfStmt final : public Stmt {
    Expr* m_cond;
    Stmt* m_then;
    Stmt* m_else;

    IfStmt(SourceSpan span, Expr* cond, Stmt* then, Stmt* els)
      : Stmt(Stmt::Kind::If, span), m_cond(cond), m_then(then), m_else(els) {}

public:
    [[nodiscard]]
    static IfStmt* create(AST::Context& ctx, SourceSpan span, Expr* cond, 
                          Stmt* then, Stmt* els);

    ~IfStmt() override;

    IfStmt(const IfStmt&) = delete;
    void operator=(const IfStmt&) = delete;

    IfStmt(IfStmt&&) noexcept = delete;
    void operator=(IfStmt&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { visitor.visit(*this); }

    const Expr* get_cond() const { return m_cond; }
    Expr* get_cond() { return m_cond; }

    const Stmt* get_then() const { return m_then; }
    Stmt* get_then() { return m_then; }

    const Stmt* get_else() const { return m_else; }
    Stmt* get_else() { return m_else; }

    bool has_else() const { return m_else != nullptr; }
};

/// Represents a `restart` statement.
class RestartStmt final : public Stmt {
    RestartStmt(SourceSpan span) : Stmt(Stmt::Kind::Restart, span) {}

public:
    [[nodiscard]]
    static RestartStmt* create(AST::Context& ctx, SourceSpan span);

    ~RestartStmt() = default;

    RestartStmt(const RestartStmt&) = delete;
    void operator=(const RestartStmt&) = delete;

    RestartStmt(RestartStmt&&) noexcept = delete;
    void operator=(RestartStmt&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { visitor.visit(*this); }
};

/// Represents a `ret` statement.
class RetStmt final : public Stmt {
    friend class SemanticAnalysis;

    /// The return expression, if there is one.
    Expr* m_expr;

    RetStmt(SourceSpan span, Expr* expr) 
      : Stmt(Stmt::Kind::Ret, span), m_expr(expr) {}

public:
    [[nodiscard]]
    static RetStmt* create(AST::Context& ctx, SourceSpan span, Expr* expr);

    ~RetStmt() override;

    RetStmt(const RetStmt&) = delete;
    void operator=(const RetStmt&) = delete;

    RetStmt(RetStmt&&) noexcept = delete;
    void operator=(RetStmt&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { visitor.visit(*this); }

    const Expr* get_expr() const { return m_expr; }
    Expr* get_expr() { return m_expr; }

    bool has_expr() const { return m_expr != nullptr; }
};

/// Represents a `stop` statement.
class StopStmt final : public Stmt {
    StopStmt(SourceSpan span) : Stmt(Stmt::Kind::Stop, span) {}

public:
    [[nodiscard]]
    static StopStmt* create(AST::Context& ctx, SourceSpan span);
    
    ~StopStmt() = default;

    StopStmt(const StopStmt&) = delete;
    void operator=(const StopStmt&) = delete;

    StopStmt(StopStmt&&) noexcept = delete;
    void operator=(StopStmt&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { visitor.visit(*this); }
};

/// Represents a `until` statement.
class UntilStmt final : public Stmt {
    Expr* m_cond;
    Stmt* m_body;

    UntilStmt(SourceSpan span, Expr* cond, Stmt* body)
      : Stmt(Stmt::Kind::Until, span), m_cond(cond), m_body(body) {}

public:
    [[nodiscard]]
    static UntilStmt* create(AST::Context& ctx, SourceSpan span, Expr* cond, 
                             Stmt* body);

    ~UntilStmt() override;

    UntilStmt(const UntilStmt&) = delete;
    void operator=(const UntilStmt&) = delete;

    UntilStmt(UntilStmt&&) noexcept = delete;
    void operator=(UntilStmt&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { visitor.visit(*this); }

    const Expr* get_cond() const { return m_cond; }
    Expr* get_cond() { return m_cond; }

    const Stmt* get_body() const { return m_body; }
    Stmt* get_body() { return m_body; }

    bool has_body() const { return m_body != nullptr; }
};

/// Represents a statement that encapsulates a rune.
class RuneStmt final : public Stmt {
    Rune* m_rune;

    RuneStmt(SourceSpan span, Rune* rune)
      : Stmt(Stmt::Kind::Rune, span), m_rune(rune) {}

public:
    [[nodiscard]] static RuneStmt* create(AST::Context& ctx, SourceSpan span, 
                                          Rune* rune);

    ~RuneStmt() override;

    RuneStmt(const RuneStmt&) = delete;
    void operator=(const RuneStmt&) = delete;

    RuneStmt(RuneStmt&&) noexcept = delete;
    void operator=(RuneStmt&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { visitor.visit(*this); }

    const Rune* get_rune() const { return m_rune; }
    Rune* get_rune() { return m_rune; }
};

} // namespace lace

#endif // LOVELACE_STMT_H_
