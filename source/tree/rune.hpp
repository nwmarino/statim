#ifndef STATIM_TREE_RUNE_HPP_
#define STATIM_TREE_RUNE_HPP_

#include "expr.hpp"

namespace stm {

class Rune : public Stmt {
    friend class SymbolAnalysis;
    friend class SemanticAnalysis;
    friend class Codegen;

public:
    enum class Kind : u8 {
        ABI, Abort, Alignas, Asm, Assert,
        Code, Comptime,
        Deprecated, Dump,
        If, Inline, Intrinsic,
        NoDiscard, NoOptimize, NoReturn, NoScope,
        Packed, Path, Public, Private, Print, Println,
        Unsafe,
        Write, Writeln
    };

private:
    Kind                kind;
    std::vector<Expr*>  args;

public:
    Rune(const Span& span, Kind kind, const std::vector<Expr*>& args);
    ~Rune() override;

    Kind get_kind() const { return kind; }

    const std::vector<Expr*>& get_args() const { return args; }

    void accept(Visitor& visitor) override {
        visitor.visit(*this);
    }

    void print(std::ostream& os) const override;
};

class RuneExpr final : public Expr {
    friend class SymbolAnalysis;
    friend class SemanticAnalysis;
    friend class Codegen;
    
    Rune* pRune;

public:
    RuneExpr(
        const Span& span, 
        const Type* pType, 
        Rune* pRune);
    
    ~RuneExpr() override;

    bool is_constant() const override;

    const Rune* get_rune() const { return pRune; }

    void accept(Visitor& visitor) override {
        visitor.visit(*this);
    }

    void print(std::ostream& os) const override;
};

} // namespace stm

#endif // STATIM_TREE_RUNE_HPP_
