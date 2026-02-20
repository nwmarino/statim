//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/tree/Defn.h"
#include "lace/tree/Expr.h"
#include "lace/tree/Type.h"

#include <cassert>

using namespace lace;

//>==---------------------------------------------------------------------------
//                          BoolLiteral Implementation
//>==---------------------------------------------------------------------------

BoolLiteral* BoolLiteral::create(AST::Context& ctx, SourceSpan span, 
                                 bool value) {
    return new BoolLiteral(
        span, 
        BuiltinType::get(ctx, BuiltinType::Kind::Bool), 
        value
    );
}

//>==---------------------------------------------------------------------------
//                          CharLiteral Implementation
//>==---------------------------------------------------------------------------

CharLiteral* CharLiteral::create(AST::Context& ctx, SourceSpan span, char value) {
    return new CharLiteral(
        span, 
        BuiltinType::get(ctx, BuiltinType::Kind::Char), 
        value
    );
}

//>==---------------------------------------------------------------------------
//                          IntegerLiteral Implementation
//>==---------------------------------------------------------------------------

IntegerLiteral* IntegerLiteral::create(AST::Context& ctx, SourceSpan span, 
                                       const QualType& type, int64_t value) {
    return new IntegerLiteral(span, type, value);
}

//>==---------------------------------------------------------------------------
//                          FloatLiteral Implementation
//>==---------------------------------------------------------------------------

FloatLiteral* FloatLiteral::create(AST::Context& ctx, SourceSpan span, 
                                   const QualType& type, double value) {
    return new FloatLiteral(span, type, value);
}

//>==---------------------------------------------------------------------------
//                          NullLiteral Implementation
//>==---------------------------------------------------------------------------

NullLiteral* NullLiteral::create(AST::Context& ctx, SourceSpan span, 
                                 const QualType& type) {
    return new NullLiteral(span, type);
}

//>==---------------------------------------------------------------------------
//                          StringLiteral Implementation
//>==---------------------------------------------------------------------------

StringLiteral* StringLiteral::create(AST::Context& ctx, SourceSpan span, 
                                     const std::string& value) {
    return new StringLiteral(
        span, 
        PointerType::get(ctx, BuiltinType::get(ctx, BuiltinType::Kind::Char)),
        value
    );
}

//>==---------------------------------------------------------------------------
//                          BinaryOp Implementation
//>==---------------------------------------------------------------------------

BinaryOp::~BinaryOp() {
    delete m_lhs;
    m_lhs = nullptr;

    delete m_rhs;
    m_rhs = nullptr;
}

BinaryOp* BinaryOp::create(AST::Context& ctx, SourceSpan span, Operator op, 
                           Expr* lhs, Expr* rhs) {
    assert(op != Unknown && "invalid operator!");
    assert(lhs && "lhs cannot be null!");
    assert(rhs && "rhs cannot be null!");
    return new BinaryOp(span, lhs->get_type(), op, lhs, rhs);
}

//>==---------------------------------------------------------------------------
//                          UnaryOp Implementation
//>==---------------------------------------------------------------------------

UnaryOp::~UnaryOp() {
    delete m_expr;
    m_expr = nullptr;
}

UnaryOp* UnaryOp::create(AST::Context& ctx, SourceSpan span, Operator op, 
                         bool prefix, Expr* expr) {
    assert(op != Unknown && "invalid operator!");
    assert(expr && "expr cannot be null!");
    return new UnaryOp(span, expr->get_type(), op, prefix, expr);
}

//>==---------------------------------------------------------------------------
//                          AccessExpr Implementation
//>==---------------------------------------------------------------------------

AccessExpr::~AccessExpr() {
    delete m_base;
    m_base = nullptr;
}

AccessExpr* AccessExpr::create(AST::Context& ctx, SourceSpan span, Expr* base, 
                               const std::string& name) {
    assert(base && "base cannot be null!");
    return new AccessExpr(
        span, 
        {},
        base, 
        name, 
        nullptr
    );
}

//>==---------------------------------------------------------------------------
//                          CallExpr Implementation
//>==---------------------------------------------------------------------------

CallExpr::~CallExpr() {
    if (m_callee)
        delete m_callee;
    
    m_callee = nullptr;
    
    for (Expr* arg : m_args) {
        if (arg)
            delete arg;
    }

    m_args.clear();
}

CallExpr* CallExpr::create(AST::Context& ctx, SourceSpan span, Expr* callee, 
                           const std::vector<Expr*>& args) {
    assert(callee && "callee cannot be null!");
    return new CallExpr(
        span, 
        callee->get_type(), 
        callee, 
        args
    );
}

//>==---------------------------------------------------------------------------
//                          CastExpr Implementation
//>==---------------------------------------------------------------------------

CastExpr::~CastExpr() {
    if (m_expr)
        delete m_expr;
    
    m_expr = nullptr;
}

CastExpr* CastExpr::create(AST::Context& ctx, SourceSpan span, 
                           const QualType& type, Expr* expr) {
    assert(expr && "expr cannot be null!");
    return new CastExpr(span, type, expr);
}

//>==---------------------------------------------------------------------------
//                          ParenExpr Implementation
//>==---------------------------------------------------------------------------

ParenExpr::~ParenExpr() {
    if (m_expr)
        delete m_expr;
    
    m_expr = nullptr;
}

ParenExpr* ParenExpr::create(AST::Context& ctx, SourceSpan span, Expr* expr) {
    assert(expr && "expr cannot be null!");
    return new ParenExpr(span, expr->get_type(), expr);
}

//>==---------------------------------------------------------------------------
//                          ParenExpr Implementation
//>==---------------------------------------------------------------------------

RefExpr* RefExpr::create(AST::Context& ctx, SourceSpan span, 
                         const std::string& name, const ValueDefn* defn) {
    return new RefExpr(span, defn ? defn->get_type() : nullptr, name, defn);
}

bool RefExpr::is_lvalue() const {
    assert(is_resolved() && "reference not resolved yet!");

    return dynamic_cast<const VariableDefn*>(m_defn) 
        || dynamic_cast<const ParameterDefn*>(m_defn);
}

//>==---------------------------------------------------------------------------
//                          SizeofExpr Implementation
//>==---------------------------------------------------------------------------

SizeofExpr* SizeofExpr::create(AST::Context& ctx, SourceSpan span, 
                               const QualType& target) {
    return new SizeofExpr(
        span, 
        BuiltinType::get(ctx, BuiltinType::Kind::UInt64), 
        target
    );
}

//>==---------------------------------------------------------------------------
//                          SubscriptExpr Implementation
//>==---------------------------------------------------------------------------

SubscriptExpr::~SubscriptExpr() {
    if (m_base)
        delete m_base;
    
    m_base = nullptr;

    if (m_index)
        delete m_index;
    
    m_index = nullptr;
}

SubscriptExpr* SubscriptExpr::create(AST::Context& ctx, SourceSpan span, 
                                     Expr* base, Expr* index) {
    assert(base && "base cannot be null!");
    assert(index && "index cannot be null!");
    return new SubscriptExpr(span, base->get_type(), base, index);
}

//>==---------------------------------------------------------------------------
//                          StructInitExpr Implementation
//>==---------------------------------------------------------------------------

StructInitExpr::~StructInitExpr() {
    for (const auto& [field, expr] : m_fields) {
        if (expr)
            delete expr;
    }

    m_fields.clear();
}

StructInitExpr* StructInitExpr::create(
        AST::Context& ctx, SourceSpan span, const QualType& type, 
        const std::map<std::string, Expr*>& fields) {
    return new StructInitExpr(span, type, fields);
}
