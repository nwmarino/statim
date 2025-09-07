#ifndef STATIM_TREE_EXPR_HPP_
#define STATIM_TREE_EXPR_HPP_

#include "tree/type.hpp"
#include "tree/stmt.hpp"

namespace stm {

class Expr : public Stmt {
    friend class SymbolAnalysis;
    friend class SemanticAnalysis;
    friend class Codegen;

protected:
    const Type* pType;

public:
    Expr(const Span& span, const Type* pType);

    virtual ~Expr() = default;

    virtual bool is_constant() const { return true; }

    /// Test if this expression can be used as an lvalue.
    virtual bool is_lvalue() const { return false; }

    virtual void accept(Visitor& visitor) = 0;

    virtual void print(std::ostream& os) const = 0;

    const Type* get_type() const { return pType; }
};

class BoolLiteral final : public Expr {
    friend class SymbolAnalysis;
    friend class SemanticAnalysis;
    friend class Codegen;

    bool value;

public:
    BoolLiteral(const Span& span, const Type* pType, bool value)
        : Expr(span, pType), value(value) {};

    bool get_value() const { return value; }

    void accept(Visitor& visitor) override {
        visitor.visit(*this);
    }

    void print(std::ostream& os) const override;
};

class IntegerLiteral final : public Expr {
    friend class SymbolAnalysis;
    friend class SemanticAnalysis;
    friend class Codegen;

    i64 value;

public:
    IntegerLiteral(const Span& span, const Type* pType, i64 value)
        : Expr(span, pType), value(value) {};
    
    i64 get_value() const { return value; }

    void accept(Visitor& visitor) override {
        visitor.visit(*this);
    }

    void print(std::ostream& os) const override;
};

class FloatLiteral final : public Expr {
    friend class SymbolAnalysis;
    friend class SemanticAnalysis;
    friend class Codegen;

    f64 value;

public:
    FloatLiteral(const Span& span, const Type* pType, f64 value)
        : Expr(span, pType), value(value) {};

    f64 get_value() const { return value; }

    void accept(Visitor& visitor) override {
        visitor.visit(*this);
    }

    void print(std::ostream& os) const override;
};

class CharLiteral final : public Expr {
    friend class SymbolAnalysis;
    friend class SemanticAnalysis;
    friend class Codegen;

    char value;

public:
    CharLiteral(const Span& span, const Type* pType, char value) 
        : Expr(span, pType), value(value) {};

    char get_value() const { return value; }

    void accept(Visitor& visitor) override {
        visitor.visit(*this);
    }

    void print(std::ostream& os) const override;
};

class StringLiteral final : public Expr {
    friend class SymbolAnalysis;
    friend class SemanticAnalysis;
    friend class Codegen;

    std::string value;

public:
    StringLiteral(const Span& span, const Type* pType, const std::string& value) 
        : Expr(span, pType), value(value) {};

    const std::string& get_value() const { return value; }

    void accept(Visitor& visitor) override {
        visitor.visit(*this);
    }

    void print(std::ostream& os) const override;
};

class NullLiteral final : public Expr {
    friend class SymbolAnalysis;
    friend class SemanticAnalysis;
    friend class Codegen;

public:
    NullLiteral(const Span& span, const Type* pType) : Expr(span, pType) {};

    void accept(Visitor& visitor) override {
        visitor.visit(*this);
    }

    void print(std::ostream& os) const override;
};

class BinaryExpr final : public Expr {
    friend class SymbolAnalysis;
    friend class SemanticAnalysis;
    friend class Codegen;

public:
    /// Different kinds of binary operators.
    enum class Operator : u8 {
        Unknown = 0x0,
        Assign,
        Add, Add_Assign,
        Sub, Sub_Assign,
        Mul, Mul_Assign,
        Div, Div_Assign,
        Mod, Mod_Assign,
        Equals, Not_Equals,
        Less_Than, Less_Than_Equals,
        Greater_Than, Greater_Than_Equals,
        Bitwise_And, Bitwise_And_Assign,
        Bitwise_Or, Bitwise_Or_Assign,
        Bitwise_Xor, Bitwise_Xor_Assign,
        Logical_And, Logical_Or,
        Left_Shift, Left_Shift_Assign,
        Right_Shift, Right_Shift_Assign,
    };

    static bool is_comparison(Operator op);

    static bool is_assignment(Operator op);

    static bool supports_ptr_arith(Operator op);

private:
    Operator    op;
    Expr*       pLeft;
    Expr*       pRight;

public:
    BinaryExpr(
        const Span& span, 
        const Type* pType, 
        Operator op, 
        Expr* pLeft, 
        Expr* pRight);

    ~BinaryExpr() override;

    bool is_constant() const override 
    { return pLeft->is_constant() && pRight->is_constant(); }

    Operator get_operator() const { return op; }

    const Expr* get_lhs() const { return pLeft; }

    const Expr* get_rhs() const { return pRight; }

    void accept(Visitor& visitor) override {
        visitor.visit(*this);
    }

    void print(std::ostream& os) const override;
};

class UnaryExpr final : public Expr {
    friend class SymbolAnalysis;
    friend class SemanticAnalysis;
    friend class Codegen;

public:
    enum class Operator : u8 {
        Unknown = 0x0,
        Increment,
        Decrement,
        Dereference,
        Address_Of,
        Negate,
        Logical_Not,
        Bitwise_Not,
    };

    static bool is_prefix(Operator op) {
        return op != Operator::Unknown;
    }

    static bool is_postfix(Operator op) {
        return op == Operator::Increment || op == Operator::Decrement;
    }

private:
    Operator    op;
    Expr*       pExpr;
    bool        postfix;

public:
    UnaryExpr(
        const Span& span, 
        const Type* pType, 
        Operator op, 
        Expr* pExpr, 
        bool postfix);
    
    ~UnaryExpr() override;

    bool is_constant() const override;

    bool is_lvalue() const override { return op == Operator::Dereference; }

    Operator get_operator() const { return op; }

    const Expr* get_expr() const { return pExpr; }

    bool is_prefix() const { return !postfix; }

    bool is_postfix() const { return postfix; }

    void accept(Visitor& visitor) override {
        visitor.visit(*this);
    }

    void print(std::ostream& os) const override;
};

class CastExpr final : public Expr {
    friend class SymbolAnalysis;
    friend class SemanticAnalysis;
    friend class Codegen;

    Expr* pExpr;

public:
    CastExpr(
        const Span& span, 
        const Type* pType, 
        Expr* pExpr);

    ~CastExpr() override;

    bool is_constant() const override { return pExpr->is_constant(); }

    const Expr* get_expr() const { return pExpr; }

    void accept(Visitor& visitor) override {
        visitor.visit(*this);
    }
    
    void print(std::ostream& os) const override;
};

class ParenExpr final : public Expr {
    friend class SymbolAnalysis;
    friend class SemanticAnalysis;
    friend class Codegen;

    Expr* pExpr;

public:
    ParenExpr(
        const Span& span, 
        Expr* pExpr);

    ~ParenExpr() override;

    bool is_constant() const override { return pExpr->is_constant(); }

    const Expr* get_expr() const { return pExpr; }

    void accept(Visitor& visitor) override {
        visitor.visit(*this);
    }

    void print(std::ostream& os) const override;
};

class SizeofExpr final : public Expr {
    friend class SymbolAnalysis;
    friend class SemanticAnalysis;
    friend class Codegen;

    const Type* pTarget;

public:
    SizeofExpr(
        const Span& span,
        const Type* pType,
        const Type* pTarget);

    const Type* get_target() const { return pTarget; }

    void accept(Visitor& visitor) override {
        visitor.visit(*this);
    }

    void print(std::ostream& os) const override;
};

class SubscriptExpr final : public Expr {
    friend class SymbolAnalysis;
    friend class SemanticAnalysis;
    friend class Codegen;

    Expr* pBase;
    Expr* pIndex;

public:
    SubscriptExpr(
        const Span& span, 
        const Type* pType,
        Expr* pBase, 
        Expr* pIndex);
    
    ~SubscriptExpr() override;

    bool is_constant() const override 
    { return pBase->is_constant() && pIndex->is_constant(); }

    bool is_lvalue() const override{ return true; }

    const Expr* get_base() const { return pBase; }

    const Expr* get_index() const { return pIndex; }

    void accept(Visitor& visitor) override {
        visitor.visit(*this);
    }

    void print(std::ostream& os) const override;
};

class ReferenceExpr : public Expr {
    friend class SymbolAnalysis;
    friend class SemanticAnalysis;
    friend class Codegen;

protected:
    std::string name;
    const Decl* pDecl;

public:
    ReferenceExpr(
        const Span& span, 
        const Type* pType, 
        const std::string& name);
    
    bool is_constant() const override { return false; }

    bool is_lvalue() const override{ return true; }

    const std::string& get_name() const { return name; }

    const Decl* get_decl() const { return pDecl; }

    void set_decl(const Decl* pDecl) { this->pDecl = pDecl; }

    void accept(Visitor& visitor) override {
        visitor.visit(*this);
    }

    void print(std::ostream& os) const override;
};

class MemberExpr final : public ReferenceExpr {
    friend class SymbolAnalysis;
    friend class SemanticAnalysis;
    friend class Codegen;

    Expr* pBase;

public:
    MemberExpr(
        const Span& span, 
        const Type* pType, 
        const std::string& member, 
        Expr* pBase);

    ~MemberExpr() override;

    bool is_constant() const override { return false; }

    bool is_lvalue() const override{ return true; }

    const Expr* get_base() const { return pBase; }

    void accept(Visitor& visitor) override {
        visitor.visit(*this);
    }

    void print(std::ostream& os) const override;
};

class CallExpr final : public ReferenceExpr {
    friend class SymbolAnalysis;
    friend class SemanticAnalysis;
    friend class Codegen;

    std::vector<Expr*> args;

public:
    CallExpr(
        const Span& span, 
        const Type* pType, 
        const std::string& callee, 
        const std::vector<Expr*> &args);

    ~CallExpr() override;

    bool is_constant() const override { return false; }

    const std::vector<Expr*>& get_args() const { return args; }

    u32 num_args() const { return args.size(); }

    void accept(Visitor& visitor) override {
        visitor.visit(*this);
    }

    void print(std::ostream& os) const override;
};

} // namespace stm

#endif // STATIM_TREE_EXPR_HPP_
