//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/tree/Defn.h"
#include "lace/tree/Expr.h"
#include "lace/tree/Type.h"

#include <cassert>

using namespace lace;

BoolLiteral* BoolLiteral::create(AST::Context& ctx, SourceSpan span, 
                                 bool value) {
    return new BoolLiteral(
        span, BuiltinType::get(ctx, BuiltinType::Kind::Bool), value);
}

CharLiteral* CharLiteral::create(AST::Context& ctx, SourceSpan span, char value) {
    return new CharLiteral(
        span, BuiltinType::get(ctx, BuiltinType::Kind::Char), value);
}

IntegerLiteral* IntegerLiteral::create(AST::Context& ctx, SourceSpan span, 
                                       const QualType& type, int64_t value) {
    return new IntegerLiteral(span, type, value);
}

FloatLiteral* FloatLiteral::create(AST::Context& ctx, SourceSpan span, 
                                   const QualType& type, double value) {
    return new FloatLiteral(span, type, value);
}

NullLiteral* NullLiteral::create(AST::Context& ctx, SourceSpan span, 
                                 const QualType& type) {
    return new NullLiteral(span, type);
}

StringLiteral* StringLiteral::create(AST::Context& ctx, SourceSpan span, 
                                     const std::string& value) {
    return new StringLiteral(
        span, 
        PointerType::get(ctx, BuiltinType::get(ctx, BuiltinType::Kind::Char)),
        value);
}

BinaryOp::~BinaryOp() {
    delete m_lhs;
    m_lhs = nullptr;

    delete m_rhs;
    m_rhs = nullptr;
}

BinaryOp* BinaryOp::create(AST::Context& ctx, SourceSpan span, Operator op, 
                           Expr* lhs, Expr* rhs) {
    assert(op != Unknown && "invalid operator!");
    assert(lhs && "invalid left operand!");
    assert(rhs && "invalid right operand!");
    return new BinaryOp(span, lhs->get_type(), op, lhs, rhs);
}

UnaryOp::~UnaryOp() {
    delete m_expr;
    m_expr = nullptr;
}

UnaryOp* UnaryOp::create(AST::Context& ctx, SourceSpan span, Operator op, 
                         bool prefix, Expr* expr) {
    assert(op != Unknown && "invalid operator!");
    assert(expr && "invalid operand!");
    return new UnaryOp(span, expr->get_type(), op, prefix, expr);
}

AccessExpr::~AccessExpr() {
    delete m_base;
    m_base = nullptr;
}

AccessExpr* AccessExpr::create(AST::Context& ctx, SourceSpan span, Expr* base, 
                               const std::string& name) {
    assert(base && "invalid access base expression!");
    return new AccessExpr(
        span, 
        {},
        base, 
        name, 
        nullptr);
}

CallExpr::~CallExpr() {
    delete m_callee;
    m_callee = nullptr;
    
    for (Expr* arg : m_args)
        delete arg;

    m_args.clear();
}

CallExpr* CallExpr::create(AST::Context& ctx, SourceSpan span, Expr* callee, 
                           const Args& args) {
    assert(callee && "invalid callee expression!");
    return new CallExpr(
        span, callee ? callee->get_type() : nullptr, callee, args);
}

CastExpr::~CastExpr() {
    delete m_expr;
    m_expr = nullptr;
}

CastExpr* CastExpr::create(AST::Context& ctx, SourceSpan span, 
                           const QualType& type, Expr* expr) {
    assert(expr && "invalid cast expression!");
    return new CastExpr(span, type, expr);
}

ParenExpr::~ParenExpr() {
    delete m_expr;
    m_expr = nullptr;
}

ParenExpr* ParenExpr::create(AST::Context& ctx, SourceSpan span, Expr* expr) {
    assert(expr && "invalid parentheses expression!");
    return new ParenExpr(span, expr->get_type(), expr);
}

RefExpr* RefExpr::create(AST::Context& ctx, SourceSpan span, 
                         const std::string& name, const ValueDefn* defn) {
    return new RefExpr(span, defn ? defn->get_type() : nullptr, name, defn);
}

bool RefExpr::is_lvalue() const {
    assert(m_defn && "reference not resolved yet!");

    return m_defn->get_kind() == Defn::Parameter 
        || m_defn->get_kind() == Defn::Variable;
}

SizeofExpr* SizeofExpr::create(AST::Context& ctx, SourceSpan span, 
                               const QualType& target) {
    return new SizeofExpr(
        span, BuiltinType::get(ctx, BuiltinType::Kind::UInt64), target);
}

SubscriptExpr::~SubscriptExpr() {
    delete m_base;
    m_base = nullptr;

    delete m_index;
    m_index = nullptr;
}

SubscriptExpr* SubscriptExpr::create(AST::Context& ctx, SourceSpan span, 
                                     Expr* base, Expr* index) {
    assert(base && "invalid base expression!");
    assert(index && "invalid index expression!");
    return new SubscriptExpr(span, base->get_type(), base, index);
}
