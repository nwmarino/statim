#ifndef STATIM_TREE_RUNE_HPP_
#define STATIM_TREE_RUNE_HPP_

#include "expr.hpp"

namespace stm {

/// Represents a rune within the syntax tree.
///
/// Runes give way to special builtin behaviours, and come in a couple flavors:
/// decorators, values, and statements. They can not and do not declare 
/// anything which is named.
///
/// Decorative runes, i.e. $no_scope or $private, modify an aspect of the node
/// they sit on. Decorative runes may be embedded directly into instances of 
/// Decl or BlockStmt.
///
/// Valued runes, i.e. $path or $comptime, evaluate to special values during
/// code generation. These kinds of runes are attached to instances of RuneExpr 
/// so they can be considered expressions by code generators.
///
/// Statement runes, i.e. $if or $asm, can provide compile-time control flow
/// with constant operands, or expand into more code similar to macros. These
/// kinds of runes are attached to instance of RuneStmt.
class Rune final {
public:
    /// Potential kinds of runes, grouped by flavor.
    enum Kind : u8 {
        Unknown = 0x0,

        /// Decorative runes.
        ABI, Alignas,
        Deprecated, Dump,
        Inline, Intrinsic,
        NoDiscard, NoOptimize, NoReturn, NoScope,
        Packed, Public, Private,
        Unsafe,

        /// Valued runes.
        Comptime,
        Path,

        /// Statement runes.
        Abort, Asm, Assert,
        If, 
        Print, Println,
        Write, Writeln,
    };

    /// Returns true if runes of the given kind are decorators, that is, not
    /// evaluable and do not modify control flow.
    static bool is_decorator(Kind kind);

    /// Returns true if runes of the given kind may be evaluated by the 
    /// compiler.
    static bool is_value(Kind kind);

    /// Returns true if runes of the given kind may manipulate control flow
    /// or expand into code.
    static bool is_statement(Kind kind);

    /// Returns true if runes of the given kind can accept an argument list
    /// enclosed by parentheses (, ).
    static bool accepts_args(Kind kind);

    /// Returns the rune equivelant of |str|. Returns `Rune::Unknown` if there 
    /// is no recognized equivelant.
    static Kind from_string(const std::string& str);

    /// Returns the string equivelant of rune |kind|.
    static std::string to_string(Kind kind);

private:
    Kind m_kind;
    std::vector<Expr*> m_args;

public:
    Rune(Kind kind, const std::vector<Expr*>& args = {});

    Rune(const Rune&) = delete;
    Rune& operator = (const Rune&) = delete;

    ~Rune();

    /// Returns the kind of rune this is.
    Kind kind() const { return m_kind; }

    /// Returns the arguments to this rune, if there are any.
    const std::vector<Expr*>& args() const { return m_args; }
    std::vector<Expr*>& args() { return m_args; }

    /// Returns the number of arguments in this rune.
    u32 num_args() const { return args().size(); }

    /// Returns true if this rune has any arguments to it.
    bool has_args() const { return !args().empty(); }

    void print(std::ostream& os) const;
};

/// Represents statement-based runes that manipulate control flow or expand
/// during code generation.
class RuneStmt final : public Stmt {
    friend class SymbolAnalysis;
    friend class SemanticAnalysis;
    friend class Codegen;
    
    Rune* m_rune;

public:
    RuneStmt(const Span& span, Rune* rune);
    
    RuneStmt(const RuneStmt&) = delete;
    RuneStmt& operator = (const RuneStmt&) = delete;

    ~RuneStmt() override;

    /// Returns the rune embedded in this expression.
    const Rune* rune() const { return m_rune; }
    Rune* rune() { return m_rune; }

    void accept(Visitor& visitor) override {
        visitor.visit(*this);
    }

    void print(std::ostream& os) const override;
};

/// Represents evaluable runes as expression nodes in the syntax tree.
class RuneExpr final : public Expr {
    friend class SymbolAnalysis;
    friend class SemanticAnalysis;
    friend class Codegen;
    
    Rune* m_rune;

public:
    RuneExpr(const Span& span, const Type* type, Rune* rune);

    RuneExpr(const RuneExpr&) = delete;
    RuneExpr& operator = (const RuneExpr&) = delete;
    
    ~RuneExpr() override;

    bool is_constant() const override { return true; }

    /// Returns the rune embedded in this expression.
    const Rune* rune() const { return m_rune; }
    Rune* rune() { return m_rune; }

    void accept(Visitor& visitor) override {
        visitor.visit(*this);
    }

    void print(std::ostream& os) const override;
};

} // namespace stm

#endif // STATIM_TREE_RUNE_HPP_
