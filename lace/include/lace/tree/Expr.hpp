//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#ifndef LOVELACE_EXPR_H_
#define LOVELACE_EXPR_H_

//
//  This header file declares a set of polymorphic classes for representing 
//  expressions in the abstract syntax tree.
//

#include "lace/tree/Stmt.hpp"
#include "lace/tree/Type.hpp"
#include "lace/tree/VisitorBase.hpp"
#include "lace/types/SourceSpan.hpp"

#include <cassert>
#include <string>
#include <vector>

namespace lace {

class Defn;
class FieldDefn;
class NamedDefn;
class Type;
class ValueDefn;

/// Base class for all expression nodes in the abstract syntax tree.
class Expr {
public:
    /// The different kinds of expressions.
    enum Kind : uint32_t {
        Bool,
        Char,
        Integer,
        Float,
        Null,
        String,
        Binary,
        Unary,
        Access,
        Call,
        Cast,
        Paren,
        Ref,
        Sizeof,
        Subscript,
    };

protected:
    /// The kind of expression this is.
    const Kind m_kind;

    /// The span of source code that this expression covers.
    const SourceSpan m_span;

    /// The type of this expression.
    QualType m_type;

    Expr(Kind kind, SourceSpan span, const QualType& type) 
      : m_kind(kind), m_span(span), m_type(type) {}

public:
    virtual ~Expr() = default;

    Expr(const Expr&) = delete;
    void operator=(const Expr&) = delete;

    Expr(Expr&&) noexcept = delete;
    void operator=(Expr&&) noexcept = delete;

    virtual void accept(VisitorBase& visitor) = 0;

    /// Test if this expression is constant i.e. is known at compile-time.
    ///
    /// Success of this function does not necessarily depend on the given
    /// expression being a literal. For example, the address of a variable
    /// is relatively known at compile-time, and thus the unary operator `&`
    /// would be considered a constant expression.
    virtual bool is_constant() const { return false; }

    /// Test if this expression may be used as an lvalue.
    ///
    /// This function does not necessarily state that a given expression *is*
    /// being used as an lvalue. Instead, the point of it is to gauge whether
    /// an expression is appopriate in place of an lvalue for the sake of
    /// semantic analysis.
    virtual bool is_lvalue() const { return false; }

    Kind get_kind() const { return m_kind; }

    /// Test if this is a boolean literal.
    bool is_bool() const { return m_kind == Bool; }

    /// Test if this is a character literal.
    bool is_char() const { return m_kind == Char; }

    /// Test if this is an integer literal.
    bool is_integer() const { return m_kind == Integer; }

    /// Test if this is a float literal.
    bool is_float() const { return m_kind == Float; }

    /// Test if this is a null literal.
    bool is_null() const { return m_kind == Null; }

    /// Test if this is a string literal.
    bool is_string() const { return m_kind == String; }

    /// Test if this is a binary operator.
    bool is_binary() const { return m_kind == Binary; }

    /// Test if this is a unary operator.
    bool is_unary() const { return m_kind == Unary; }

    /// Test if this is an access expression.
    bool is_access() const { return m_kind == Access; }

    /// Test if this is a call expression.
    bool is_call() const { return m_kind == Call; }

    /// Test if this is a cast expression.
    bool is_cast() const { return m_kind == Cast; }

    /// Test if this is a parentheses expression.
    bool is_paren() const { return m_kind == Paren; }

    /// Test if this is a reference expression.
    bool is_ref() const { return m_kind == Ref; }

    /// Test if this is a sizeof expression.
    bool is_sizeof() const { return m_kind == Sizeof; }

    /// Test if this is a subscript expression.
    bool is_subscript() const { return m_kind == Subscript; }

    SourceSpan get_span() const { return m_span; }

    void set_type(const QualType& type) { m_type = type; }
    const QualType& get_type() const { return m_type; }
    QualType& get_type() { return m_type; }
};

/// Representation of boolean literals, e.g. 'true' or 'false'.
class BoolLiteral final : public Expr {
    const bool m_value;

    BoolLiteral(SourceSpan span, const QualType& type, bool value)
      : Expr(Expr::Bool, span, type), m_value(value) {}

public:
    [[nodiscard]]
    static BoolLiteral* create(AST::Context& ctx, SourceSpan span, bool value);

    ~BoolLiteral() = default;

    BoolLiteral(const BoolLiteral&) = delete;
    void operator=(const BoolLiteral&) = delete;

    BoolLiteral(BoolLiteral&&) noexcept = delete;
    void operator=(BoolLiteral&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { visitor.visit(*this); }

    bool is_constant() const override { return true; }

    bool get_value() const { return m_value; }
};

/// Representation of character literals, e.g. `'a'` and `'0'`.
class CharLiteral final : public Expr {
    char m_value;

    CharLiteral(SourceSpan span, const QualType& type, char value)
      : Expr(Expr::Char, span, type), m_value(value) {}

public:
    [[nodiscard]]
    static CharLiteral* create(AST::Context& ctx, SourceSpan span, char value);

    ~CharLiteral() = default;
    
    CharLiteral(const CharLiteral&) = delete;
    void operator=(const CharLiteral&) = delete;

    CharLiteral(CharLiteral&&) noexcept = delete;
    void operator=(CharLiteral&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { visitor.visit(*this); }

    bool is_constant() const override { return true; }

    char get_value() const { return m_value; }
};

/// Representation of integer literals, e.g. `0` and `1`.
class IntegerLiteral final : public Expr {
    const int64_t m_value;

    IntegerLiteral(SourceSpan span, const QualType& type, int64_t value)
      : Expr(Expr::Integer, span, type), m_value(value) {}

public:
    [[nodiscard]]
    static IntegerLiteral* create(AST::Context& ctx, SourceSpan span, 
                                  const QualType& type, int64_t value);

    ~IntegerLiteral() = default;
    
    IntegerLiteral(const IntegerLiteral&) = delete;
    void operator=(const IntegerLiteral&) = delete;

    IntegerLiteral(IntegerLiteral&&) noexcept = delete;
    void operator=(IntegerLiteral&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { visitor.visit(*this); }

    bool is_constant() const override { return true; }

    int64_t get_value() const { return m_value; }
};

/// Representation of floating point literals, e.g. `1.` and `3.14`.
class FloatLiteral final : public Expr {
    const double m_value;

    FloatLiteral(SourceSpan span, const QualType& type, double value)
      : Expr(Expr::Float, span, type), m_value(value) {}

public:
    [[nodiscard]]
    static FloatLiteral* create(AST::Context& ctx, SourceSpan span, 
                                const QualType& type, double value);

    ~FloatLiteral() = default;

    FloatLiteral(const FloatLiteral&) = delete;
    void operator=(const FloatLiteral&) = delete;

    FloatLiteral(FloatLiteral&&) noexcept = delete;
    void operator=(FloatLiteral&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { visitor.visit(*this); }

    bool is_constant() const override { return true; }

    double get_value() const { return m_value; }
};

/// Representation of null literals, e.g. `null`.
class NullLiteral final : public Expr {
    NullLiteral(SourceSpan span, const QualType& type) 
      : Expr(Expr::Null, span, type) {}

public:
    [[nodiscard]]
    static NullLiteral* create(AST::Context& ctx, SourceSpan span, 
                               const QualType& type);

    ~NullLiteral() = default;

    NullLiteral(const NullLiteral&) = delete;
    void operator=(const NullLiteral&) = delete;

    NullLiteral(NullLiteral&&) noexcept = delete;
    void operator=(NullLiteral&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { visitor.visit(*this); }

    bool is_constant() const override { return true; }
};

/// Representation of string literals, e.g. `"hello"` and `"world"`.
class StringLiteral final : public Expr {
    const std::string m_value;

    StringLiteral(SourceSpan span, const QualType& type, const
                  std::string& value)
      : Expr(Expr::String, span, type), m_value(value) {}

public:
    [[nodiscard]]
    static StringLiteral* create(AST::Context& ctx, SourceSpan span, 
                                 const std::string& value);

    ~StringLiteral() = default;

    StringLiteral(const StringLiteral&) = delete;
    void operator=(const StringLiteral&) = delete;

    StringLiteral(StringLiteral&&) noexcept = delete;
    void operator=(StringLiteral&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { visitor.visit(*this); }

    bool is_constant() const override { return true; }

    const std::string& get_value() const { return m_value; }
};

/// Represents a binary operation.
class BinaryOp final : public Expr {
    friend class SemanticAnalysis;

public:
    /// The different kinds of binary operators.
    enum Operator : uint32_t {
        Unknown = 0,
        Assign,
        Add,
        Sub,
        Mul,
        Div,
        Mod,
        And,
        Or,
        Xor,
        LShift,
        RShift,
        LogicAnd,
        LogicOr,
        Eq,
        NEq,
        Lt,
        LtEq,
        Gt,
        GtEq,
    };

    /// Test if the given |op| is the assignment operator `=`.
    static bool is_assignment(Operator op) { return op == Assign; }

    /// Test if the given |op| performs any sort of comparison that must
    /// result in a boolean value.
    static bool is_comparison(Operator op) {
        return is_numerical_comparison(op) || is_logical_comparison(op);
    }

    /// Test if the given |op| performs a numerical comparison.
    static bool is_numerical_comparison(Operator op) {
        return Eq <= op && op <= GtEq;
    }

    /// Test if the given |op| performs a bitwise comparison.
    static bool is_bitwise_comparison(Operator op) {
        return op == And || op == Or || op == Xor;
    }

    /// Test if the given |op| performs a logical comparison.
    static bool is_logical_comparison(Operator op) {
        return op == LogicAnd || op == LogicOr;
    }

private:
    Operator m_op;
    Expr* m_lhs;
    Expr* m_rhs;

    BinaryOp(SourceSpan span, const QualType& type, Operator op, 
             Expr* lhs, Expr* rhs)
      : Expr(Expr::Binary, span, type), m_op(op), m_lhs(lhs), m_rhs(rhs) {}

public:
    [[nodiscard]]
    static BinaryOp* create(AST::Context& ctx, SourceSpan span, Operator op, 
                            Expr* lhs, Expr* rhs);

    ~BinaryOp() override;
    
    BinaryOp(const BinaryOp&) = delete;
    void operator=(const BinaryOp&) = delete;

    BinaryOp(BinaryOp&&) noexcept = delete;
    void operator=(BinaryOp&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { visitor.visit(*this); }

    bool is_constant() const override { 
        return m_lhs->is_constant() && m_rhs->is_constant(); 
    }

    Operator get_operator() const { return m_op; }

    const Expr* get_lhs() const { return m_lhs; }
    Expr* get_lhs() { return m_lhs; }

    const Expr* get_rhs() const { return m_rhs; }
    Expr* get_rhs() { return m_rhs; }
};

/// Represents a unary operation.
class UnaryOp final : public Expr {
public:
    /// The different kinds of unary operators.
    enum Operator : uint32_t {
        Unknown = 0,
        Negate,
        Not,
        LogicNot,
        AddressOf,
        Dereference,
    };

    /// Test if the given |op| can be used as a prefix operator.
    static bool is_prefix(Operator op) {
        return op != Unknown;
    }

    /// Test if the given |op| can be used as a prefix operator.
    static bool is_postfix(Operator op) {
        return false;
    }

private:
    /// The operator to use.
    Operator m_op;

    /// If true, then this operator is being used as a prefix. Otherwise, it
    /// is being used as postfix.
    bool m_prefix;

    /// The expression to operate on.
    Expr* m_expr;

    UnaryOp(SourceSpan span, const QualType& type, Operator op, bool prefix, 
            Expr* expr)
      : Expr(Expr::Unary, span, type), m_op(op), m_prefix(prefix), 
        m_expr(expr) {}

public:
    [[nodiscard]]
    static UnaryOp* create(AST::Context& ctx, SourceSpan span, Operator op, 
                           bool prefix, Expr* expr);

    ~UnaryOp() override;

    UnaryOp(const UnaryOp&) = delete;
    void operator=(const UnaryOp&) = delete;

    UnaryOp(UnaryOp&&) noexcept = delete;
    void operator=(UnaryOp&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { visitor.visit(*this); }

    bool is_constant() const override { 
        return m_expr->is_constant() || m_op == AddressOf; 
    }

    bool is_lvalue() const override { return m_op == Dereference; }

    Operator get_operator() const { return m_op; }

    /// Test if this unary operation is interpreted as a prefix operator.
    bool is_prefix() const { return m_prefix; }

    /// Test if this unary operation is interpreted as a postfix operator.
    bool is_postfix() const { return !m_prefix; }

    const Expr* get_expr() const { return m_expr; }
    Expr* get_expr() { return m_expr; }
};

/// Represents a structure field access `.` expression.
class AccessExpr final : public Expr {
    /// The base expression i.e. structure to access a field from.
    Expr* m_base;
    
    ///  The name of the structure field to access. Used for the sake of 
    /// forwarding referencing.
    std::string m_name;

    /// The field to access.
    const FieldDefn* m_field;

    AccessExpr(SourceSpan span, const QualType& type, Expr* base, 
               const std::string& name, const FieldDefn* field)
      : Expr(Expr::Access, span, type), m_base(base), m_name(name), 
        m_field(field) {}

public:
    [[nodiscard]]
    static AccessExpr* create(AST::Context& ctx, SourceSpan span, Expr* base, 
                              const std::string& name);

    ~AccessExpr() override;

    AccessExpr(const AccessExpr&) = delete;
    void operator=(const AccessExpr&) = delete;

    AccessExpr(AccessExpr&&) = delete;
    void operator=(AccessExpr&&) = delete;

    void accept(VisitorBase& visitor) override { visitor.visit(*this); }

    bool is_lvalue() const override { return true; }

    const Expr* get_base() const { return m_base; }
    Expr* get_base() { return m_base; }

    const std::string& get_name() { return m_name; }

    void set_field(const FieldDefn* field) { m_field = field; }
    const FieldDefn* get_field() const { return m_field; }
};

/// Represents a function call `...(...)` expression.
class CallExpr final : public Expr {
    friend class SemanticAnalysis;

public:
    using Args = std::vector<Expr*>;

private:
    Expr* m_callee;
    Args m_args;

    CallExpr(SourceSpan span, const QualType& type, Expr* callee, 
             const Args& args)
      : Expr(Expr::Call, span, type), m_callee(callee), m_args(args) {}

public:
    [[nodiscard]]
    static CallExpr* create(AST::Context& ctx, SourceSpan span, Expr* callee, 
                            const Args& args);

    ~CallExpr() override;
    
    CallExpr(const CallExpr&) = delete;
    void operator=(const CallExpr&) = delete;

    CallExpr(CallExpr&&) noexcept = delete;
    void operator=(CallExpr&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { visitor.visit(*this); }

    const Expr* get_callee() const { return m_callee; }
    Expr* get_callee() { return m_callee; }

    const Args& get_args() const { return m_args; }
    Args& get_args() { return m_args; }

    const Expr* get_arg(uint32_t i) const { 
        assert(i < num_args() && "index out of bounds!");
        return m_args[i]; 
    }

    Expr* get_arg(uint32_t i) { 
        assert(i < num_args() && "index out of bounds!");
        return m_args[i]; 
    }

    uint32_t num_args() const { return m_args.size(); }
    bool has_args() const { return !m_args.empty(); }
};

/// Represents a cast expression, i.e. `cast<T>(...)`.
class CastExpr final : public Expr {
    Expr* m_expr;

    CastExpr(SourceSpan span, const QualType& type, Expr* expr)
      : Expr(Expr::Cast, span, type), m_expr(expr) {}

public:
    [[nodiscard]]
    static CastExpr* create(AST::Context& ctx, SourceSpan span, 
                            const QualType& type, Expr* expr);

    ~CastExpr() override;
    
    CastExpr(const CastExpr&) = delete;
    void operator=(const CastExpr&) = delete;

    CastExpr(CastExpr&&) noexcept = delete;
    void operator=(CastExpr&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { visitor.visit(*this); }

    bool is_constant() const override { return m_expr->is_constant(); }

    const Expr* get_expr() const { return m_expr; }
    Expr* get_expr() { return m_expr; }
};

/// Represents an expression within parentheses, i.e. `(...)`.
class ParenExpr final : public Expr {
    Expr* m_expr;

    ParenExpr(SourceSpan span, const QualType& type, Expr* expr)
      : Expr(Expr::Paren, span, type), m_expr(expr) {}

public:
    [[nodiscard]]
    static ParenExpr* create(AST::Context& ctx, SourceSpan span, Expr* expr);

    ~ParenExpr() override;

    ParenExpr(const ParenExpr&) = delete;
    void operator=(const ParenExpr&) = delete;

    ParenExpr(ParenExpr&&) noexcept = delete;
    void operator=(ParenExpr&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { visitor.visit(*this); }

    bool is_constant() const override { return m_expr->is_constant(); }

    const Expr* get_expr() const { return m_expr; }
    Expr* get_expr() { return m_expr; }
};

/// Represents a named definition reference expression.
class RefExpr final : public Expr {
    const std::string m_name;
    const ValueDefn* m_defn;

public:
    RefExpr(SourceSpan span, const QualType& type, const std::string& name, 
                const ValueDefn* defn)
      : Expr(Expr::Ref, span, type), m_name(name), m_defn(defn) {}

public:
    [[nodiscard]]
    static RefExpr* create(AST::Context& ctx, SourceSpan span, 
                           const std::string& name, const ValueDefn* defn);

    ~RefExpr() = default;
    
    RefExpr(const RefExpr&) = delete;
    void operator=(const RefExpr&) = delete;

    RefExpr(RefExpr&&) noexcept = delete;
    void operator=(RefExpr&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { visitor.visit(*this); }

    bool is_lvalue() const override;

    const std::string& get_name() const { return m_name; }

    void set_defn(const ValueDefn* defn) { m_defn = defn; }
    const ValueDefn* get_defn() const { return m_defn; }
};

/// Represents a `sizeof(T)` expression.
class SizeofExpr final : public Expr {
    /// The type to get the size of.
    QualType m_target;

    SizeofExpr(SourceSpan span, const QualType& type, const QualType& target)
      : Expr(Expr::Sizeof, span, type), m_target(target) {}

public:
    [[nodiscard]]
    static SizeofExpr* create(AST::Context& ctx, SourceSpan span, 
                              const QualType& target);

    ~SizeofExpr() = default;
    
    SizeofExpr(const SizeofExpr&) = delete;
    void operator = (const SizeofExpr&) = delete;

    SizeofExpr(SizeofExpr&&) noexcept = delete;
    void operator=(SizeofExpr&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { visitor.visit(*this); }

    bool is_constant() const override { return true; }

    const QualType& get_target_type() const { return m_target; }
    QualType& get_target_type() { return m_target; }
};

/// Represents a subscript `[]` expression.
class SubscriptExpr final : public Expr {
    Expr* m_base;
    Expr* m_index;

    SubscriptExpr(SourceSpan span, const QualType& type, Expr* base, 
                  Expr* index)
      : Expr(Expr::Subscript, span, type), m_base(base), m_index(index) {}

public:
    [[nodiscard]]
    static SubscriptExpr* create(AST::Context& ctx, SourceSpan span, Expr* base, 
                                 Expr* index);

    ~SubscriptExpr() override;
    
    SubscriptExpr(const SubscriptExpr&) = delete;
    void operator=(const SubscriptExpr&) = delete;

    SubscriptExpr(SubscriptExpr&&) noexcept = delete;
    void operator=(SubscriptExpr&&) noexcept = delete;

    void accept(VisitorBase& visitor) override { visitor.visit(*this); }

    bool is_lvalue() const override { return true; }

    const Expr* get_base() const { return m_base; }
    Expr* get_base() { return m_base; }

    const Expr* get_index() const { return m_index; }
    Expr* get_index() { return m_index; }
};

} // namespace lace

#endif // LOVELACE_EXPR_H_
