#ifndef STATIM_AST_HPP_
#define STATIM_AST_HPP_

#include "input_file.hpp"
#include "source_loc.hpp"
#include "type.hpp"

#include <ostream>
#include <pthread.h>
#include <string>
#include <unordered_map>

namespace stm {

class Expr;
class Rune;
class Stmt;
class ParameterDecl;

class TypeContext final {
    friend class Root;
    friend class DeferredType;
    friend class FunctionType;
    friend class PointerType;

    std::unordered_map<BuiltinType::Kind, BuiltinType*> builtins {};
    std::unordered_map<const Type*, PointerType*>       pointers {};
    std::vector<DeferredType*>                          deferred {};
    std::vector<FunctionType*>                          functions {};

    const BuiltinType* get(BuiltinType::Kind kind) const;
    const PointerType* get(const Type* pPointee);
    const DeferredType* get(const DeferredType::Context& context);
    const FunctionType* get(const Type* pReturn, const std::vector<const Type*> &params);

public:
    TypeContext();
    ~TypeContext();

    TypeContext(const TypeContext&) = delete;
    TypeContext& operator = (const TypeContext&) = delete;
};

class Root final {
    InputFile&                  file;
    TypeContext                 context;
    Scope*                      pScope;
    std::vector<Decl*>          decls;
    std::vector<const Decl*>    imports;
    std::vector<const Decl*>    exports;

public:
    Root(InputFile& file, Scope* pScope);
    
    ~Root();

    InputFile& get_file() { return file; }

    TypeContext& get_context() { return context; }

    Scope* get_scope() { return pScope; }

    const Scope* get_scope() const { return pScope; }

    void add_decl(Decl* pDecl) { decls.push_back(pDecl); }

    const std::vector<const Decl*>& get_imports() const { return imports; }

    const std::vector<const Decl*>& get_exports() const { return exports; }

    const BuiltinType* get_void_type() const 
    { return context.get(BuiltinType::Kind::Void); }
    
    const BuiltinType* get_bool_type() const 
    { return context.get(BuiltinType::Kind::Bool); }

    const BuiltinType* get_char_type() const 
    { return context.get(BuiltinType::Kind::Char); }

    const BuiltinType* get_si8_type() const 
    { return context.get(BuiltinType::Kind::SInt8); }

    const BuiltinType* get_si16_type() const 
    { return context.get(BuiltinType::Kind::SInt16); }

    const BuiltinType* get_si32_type() const 
    { return context.get(BuiltinType::Kind::SInt32); }

    const BuiltinType* get_si64_type() const 
    { return context.get(BuiltinType::Kind::SInt64); }

    const BuiltinType* get_ui8_type() const 
    { return context.get(BuiltinType::Kind::UInt8); }

    const BuiltinType* get_ui16_type() const 
    { return context.get(BuiltinType::Kind::UInt16); }

    const BuiltinType* get_ui32_type() const 
    { return context.get(BuiltinType::Kind::UInt32); }

    const BuiltinType* get_ui64_type() const 
    { return context.get(BuiltinType::Kind::UInt64); }

    const BuiltinType* get_fp32_type() const 
    { return context.get(BuiltinType::Kind::Float32); }

    const BuiltinType* get_fp64_type() const 
    { return context.get(BuiltinType::Kind::Float64); }

    void print(std::ostream& os) const;
};

class Decl {
protected:
    Span                span;
    std::string         name;
    std::vector<Rune*>  decorators {};

public:
    Decl(const Span& span, const std::string& name, const std::vector<Rune*>& decorators);

    virtual ~Decl() = default;

    virtual void print(std::ostream& os) const = 0;

    const Span& get_span() const { return span; }

    const std::string& get_name() const { return name; }

    const std::vector<Rune*>& get_decorators() const { return decorators; }
};

class FunctionDecl final : public Decl {
    const FunctionType*         pType;
    std::vector<ParameterDecl*> params; 
    Scope*                      pScope;
    Stmt*                       pBody;

public:
    FunctionDecl(
        const Span& span, 
        const std::string& name, 
        const std::vector<Rune*>& decorators, 
        const FunctionType* pType, 
        const std::vector<ParameterDecl*>& params,
        Scope* pScope,
        Stmt* pBody);

    ~FunctionDecl() override;

    const FunctionType* get_type() const { return pType; }

    const std::vector<ParameterDecl*>& get_params() const { return params; }

    const ParameterDecl* get_param(u32 idx) { return params.at(idx); }

    u32 num_params() const { return params.size(); }

    const Scope* get_scope() const { return pScope; }

    const Stmt* get_body() const { return pBody; }

    bool returns() const { return pType->get_return_type()->is_void(); }

    bool has_params() const { return params.empty(); }

    bool has_body() const { return pBody != nullptr; }

    void print(std::ostream& os) const override;
};

class ParameterDecl final : public Decl {
    const Type* pType;

public:
    ParameterDecl(
        const Span& span,
        const std::string& name,
        const std::vector<Rune*>& decorators,
        const Type* pType);

    const Type* get_type() const { return pType; }

    void print(std::ostream& os) const override;
};

class VariableDecl final : public Decl {
    const Type* pType;
    Expr*       pInit;

public:
    VariableDecl(
        const Span& span,
        const std::string& name,
        const std::vector<Rune*>& decorators,
        const Type* pType);

    ~VariableDecl() override;

    const Type* get_type() const { return pType; }

    const Expr* get_init() const { return pInit; }

    bool has_init() const { return pInit != nullptr; }

    void print(std::ostream& os) const override;
};

class Stmt {
protected:
    Span span;

public:
    Stmt(const Span& span) : span(span) {};

    virtual ~Stmt() = default;

    const Span& get_span() const { return span; }

    virtual void print(std::ostream& os) const = 0;
};

class BlockStmt final : public Stmt {
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

    const Scope* get_scope() const { return pScope; }

    bool is_empty() const { return stmts.empty(); }

    void print(std::ostream& os) const override;
};

class BreakStmt final : public Stmt {
public:
    BreakStmt(const Span& span) : Stmt(span) {};

    void print(std::ostream& os) const override;
};

class ContinueStmt final : public Stmt {
public:
    ContinueStmt(const Span& span) : Stmt(span) {};

    void print(std::ostream& os) const override;
};

class DeclStmt final : public Stmt {
    Decl* pDecl;

public:
    DeclStmt(const Span& span, Decl* pDecl);
    ~DeclStmt() override;

    const Decl* get_decl() const { return pDecl; }

    void print(std::ostream& os) const override;
};

class IfStmt final : public Stmt {
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

    void print(std::ostream& os) const override;
};

class WhileStmt final : public Stmt {
    Expr* pCond;
    Stmt* pBody;

public:
    WhileStmt(const Span& span, Expr* pCond, Stmt* pBody);
    ~WhileStmt() override;

    const Expr* get_cond() const { return pCond; }

    const Stmt* get_body() const { return pBody; }

    bool has_body() const { return pBody != nullptr; }

    void print(std::ostream& os) const override;
};

class RetStmt final : public Stmt {
    Expr* pExpr;

public:
    RetStmt(const Span& span, Expr* pExpr);
    ~RetStmt() override;

    const Expr* get_expr() const { return pExpr; }

    bool has_expr() const { return pExpr != nullptr; }

    void print(std::ostream& os) const override;
};

class Rune : public Stmt {
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

    void print(std::ostream& os) const override;
};

class Expr : public Stmt {
public:
    enum class ValueKind : u8 {
        LValue, RValue
    };

protected:
    const Type* pType;
    ValueKind   vkind;

public:
    Expr(const Span& span, const Type* pType, ValueKind vkind);

    virtual ~Expr() = default;

    virtual bool is_constant() const { return true; }

    virtual void print(std::ostream& os) const = 0;

    const Type* get_type() const { return pType; }

    ValueKind get_value_kind() const { return vkind; }
};

class BoolLiteral final : public Expr {
    bool value;

public:
    BoolLiteral(const Span& span, const Type* pType, bool value)
        : Expr(span, pType, ValueKind::RValue), value(value) {};

    bool get_value() const { return value; }

    void print(std::ostream& os) const override;
};

class IntegerLiteral final : public Expr {
    i64 value;

public:
    IntegerLiteral(const Span& span, const Type* pType, i64 value)
        : Expr(span, pType, ValueKind::RValue), value(value) {};
    
    i64 get_value() const { return value; }

    void print(std::ostream& os) const override;
};

class FloatLiteral final : public Expr {
    f64 value;

public:
    FloatLiteral(const Span& span, const Type* pType, f64 value)
        : Expr(span, pType, ValueKind::RValue), value(value) {};

    f64 get_value() const { return value; }

    void print(std::ostream& os) const override;
};

class CharLiteral final : public Expr {
    char value;

public:
    CharLiteral(const Span& span, const Type* pType, char value) 
        : Expr(span, pType, ValueKind::RValue), value(value) {};

    char get_value() const { return value; }

    void print(std::ostream& os) const override;
};

class StringLiteral final : public Expr {
    std::string value;

public:
    StringLiteral(const Span& span, const Type* pType, const std::string& value) 
        : Expr(span, pType, ValueKind::RValue), value(value) {};

    const std::string& get_value() const { return value; }

    void print(std::ostream& os) const override;
};

class NilLiteral final : public Expr {
public:
    NilLiteral(const Span& span, const Type* pType) 
        : Expr(span, pType, ValueKind::RValue) {};

    void print(std::ostream& os) const override;
};

class BinaryExpr final : public Expr {
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

private:
    Operator    op;
    Expr*       pLeft;
    Expr*       pRight;

public:
    BinaryExpr(
        const Span& span, 
        const Type* pType, 
        ValueKind vkind, 
        Operator op, 
        Expr* pLeft, 
        Expr* pRight);

    ~BinaryExpr() override;

    bool is_constant() const override 
    { return pLeft->is_constant() && pRight->is_constant(); }

    Operator get_operator() const { return op; }

    const Expr* get_lhs() const { return pLeft; }

    const Expr* get_rhs() const { return pRight; }

    void print(std::ostream& os) const override;
};

class UnaryExpr final : public Expr {
public:
    enum class Operator : u8 {
        Unknown = 0x0,
        Increment,
        Decrement,
        Dereference,
        Address_Of,
        Logical_Not,
        Bitwise_Not,
    };

    static bool is_prefix(Operator op) {
        return op != Operator::Unknown;
    }

    static bool is_postfix(Operator op) {
        switch (op) {
        case Operator::Increment:
        case Operator::Decrement:
            return true;
        default:
            return false;
        }
    }

private:
    Operator    op;
    Expr*       pExpr;
    bool        postfix;

public:
    UnaryExpr(
        const Span& span, 
        const Type* pType, 
        ValueKind vkind, 
        Operator op, 
        Expr* pExpr, 
        bool postfix);
    
    ~UnaryExpr() override;

    bool is_constant() const override;

    Operator get_operator() const { return op; }

    const Expr* get_expr() const { return pExpr; }

    bool is_prefix() const { return !postfix; }

    bool is_postfix() const { return postfix; }

    void print(std::ostream& os) const override;
};

class CastExpr final : public Expr {
    Expr* pExpr;

public:
    CastExpr(
        const Span& span, 
        const Type* pType, 
        ValueKind vkind, 
        Expr* pExpr);

    ~CastExpr() override;

    bool is_constant() const override { return pExpr->is_constant(); }

    const Expr* get_expr() const { return pExpr; }
    
    void print(std::ostream& os) const override;
};

class ParenExpr final : public Expr {
    Expr* pExpr;

public:
    ParenExpr(
        const Span& span, 
        Expr* pExpr);

    ~ParenExpr() override;

    bool is_constant() const override { return pExpr->is_constant(); }

    const Expr* get_expr() const { return pExpr; }

    void print(std::ostream& os) const override;
};

class SizeofExpr final : public Expr {
    const Type* pTarget;

public:
    SizeofExpr(
        const Span& span,
        const Type* pType,
        const Type* pTarget);

    const Type* get_target() const { return pTarget; }

    void print(std::ostream& os) const override;
};

class SubscriptExpr final : public Expr {
    Expr* pBase;
    Expr* pIndex;

public:
    SubscriptExpr(
        const Span& span, 
        const Type* pType,
        ValueKind vkind, 
        Expr* pBase, 
        Expr* pIndex);
    
    ~SubscriptExpr() override;

    bool is_constant() const override { return pBase->is_constant() && pIndex->is_constant(); }

    const Expr* get_base() const { return pBase; }

    const Expr* get_index() const { return pIndex; }

    void print(std::ostream& os) const override;
};

class ReferenceExpr : public Expr {
protected:
    std::string ref;

public:
    ReferenceExpr(
        const Span& span, 
        const Type* pType, 
        ValueKind vkind, 
        const std::string& ref);
    
    bool is_constant() const override { return false; }

    const std::string& get_reference() const { return ref; }

    void print(std::ostream& os) const override;
};

class MemberExpr final : public ReferenceExpr {
    Expr* pBase;

public:
    MemberExpr(
        const Span& span, 
        const Type* pType, 
        ValueKind vkind, 
        const std::string& member, 
        Expr* pBase);

    ~MemberExpr() override;

    bool is_constant() const override { return false; }

    const Expr* get_base() const { return pBase; }

    void print(std::ostream& os) const override;
};

class CallExpr final : public ReferenceExpr {
    std::vector<Expr*> args;

public:
    CallExpr(
        const Span& span, 
        const Type* pType, 
        const std::string& callee, 
        const std::vector<Expr*> &args);

    ~CallExpr() override;

    bool is_constant() const override { return false; }

    const std::vector<Expr*>& get_arguments() const { return args; }

    void print(std::ostream& os) const override;
};

class RuneExpr final : public Expr {
    Rune* pRune;

public:
    RuneExpr(
        const Span& span, 
        const Type* pType, 
        ValueKind vkind, 
        Rune* pRune);
    
    ~RuneExpr() override;

    bool is_constant() const override;

    const Rune* get_rune() const { return pRune; }

    void print(std::ostream& os) const override;
};

} // namespace stm

#endif // STATIM_AST_HPP_
