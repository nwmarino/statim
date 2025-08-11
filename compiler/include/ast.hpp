#ifndef STATIM_AST_HPP_
#define STATIM_AST_HPP_

#include "source_loc.hpp"
#include "type.hpp"

#include <string>

namespace stm {

class Expr;

class Root final {
    struct Types final {
        BuiltinType*                pVoid;
        BuiltinType*                pBool;
        BuiltinType*                pChar;
        BuiltinType*                pSI8;
        BuiltinType*                pSI16;
        BuiltinType*                pSI32;
        BuiltinType*                pSI64;
        BuiltinType*                pUI8;
        BuiltinType*                pUI16;
        BuiltinType*                pUI32;
        BuiltinType*                pUI64;
        BuiltinType*                pFP32;
        BuiltinType*                pFP64;
        std::vector<DeferredType*>  deferred;
        std::vector<FunctionType*>  signatures;
    } types;
};

class Decl {
protected:
    Span        span;
    std::string name;

public:
    const Span& get_span() const { return span; }

    const std::string& get_name() const { return name; }
};

class FunctionDecl final : public Decl {

};

class Stmt {
protected:
    Span span;

public:
    Stmt(const Span& span) : span(span) {};

    const Span& get_span() const { return span; }
};

class BlockStmt final : public Stmt {

};

class BreakStmt final : public Stmt {

};

class ContinueStmt final : public Stmt {

};

class DeclStmt final : public Stmt {

};

class IfStmt final : public Stmt {

};

class ForStmt final : public Stmt {

};

class WhileStmt final : public Stmt {

};

class RetStmt final : public Stmt {
    Expr* pExpr;

public:
};

class Expr : public Stmt {
public:
    enum class Props : u8 {
        LValue, RValue, Initializer
    };

protected:
    const Type* pType;
    Props       props;

public:
    Expr(const Span& span, const Type* pType, Props props) 
        : Stmt(span), pType(pType), props(props) {};

    virtual ~Expr() = default;

    const Type* get_type() const { return pType; }

    Props get_properties() const { return props; }
};

class BoolLiteral final : public Expr {
    bool value;
};

class IntegerLiteral final : public Expr {
    i64 value;
};

class FloatLiteral final : public Expr {
    double value;
};

class CharLiteral final : public Expr {
    char value;
};

class StringLiteral final : public Expr {
    std::string value;
};

class BinaryExpr final : public Expr {

};

class UnaryExpr final : public Expr {

};

class CastExpr final : public Expr {

};

class ParenExpr final : public Expr {

};

class SubscriptExpr final : public Expr {

};

class ReferenceExpr : public Expr {

};

class MemberExpr final : public ReferenceExpr {

};

class CallExpr final : public ReferenceExpr {

};

} // namespace stm

#endif // STATIM_AST_HPP_
