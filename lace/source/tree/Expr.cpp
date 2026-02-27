//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/tree/Defn.h"
#include "lace/tree/Expr.h"
#include "lace/tree/Type.h"
#include "lace/tree/VisitorBase.h"

#include <cassert>

using namespace lace;

//>==---------------------------------------------------------------------------
//                          BoolLiteral Implementation
//>==---------------------------------------------------------------------------

BoolLiteral* BoolLiteral::create(Rib& rib, SourceSpan span, bool value) {
    return new BoolLiteral(
        span, 
        BuiltinType::get(rib, BuiltinType::Kind::Bool), 
        value
    );
}

//>==---------------------------------------------------------------------------
//                          CharLiteral Implementation
//>==---------------------------------------------------------------------------

CharLiteral* CharLiteral::create(Rib& rib, SourceSpan span, char value) {
    return new CharLiteral(
        span, 
        BuiltinType::get(rib, BuiltinType::Kind::Char), 
        value
    );
}

//>==---------------------------------------------------------------------------
//                          IntegerLiteral Implementation
//>==---------------------------------------------------------------------------

IntegerLiteral* IntegerLiteral::create(Rib& rib, SourceSpan span, Type* type, 
                                       int64_t value) {
    return new IntegerLiteral(span, type, value);
}

//>==---------------------------------------------------------------------------
//                          FloatLiteral Implementation
//>==---------------------------------------------------------------------------

FloatLiteral* FloatLiteral::create(Rib& rib, SourceSpan span, Type* type, 
                                   double value) {
    return new FloatLiteral(span, type, value);
}

//>==---------------------------------------------------------------------------
//                          NullLiteral Implementation
//>==---------------------------------------------------------------------------

NullLiteral* NullLiteral::create(Rib& rib, SourceSpan span, Type* type) {
    return new NullLiteral(span, type);
}

//>==---------------------------------------------------------------------------
//                          StringLiteral Implementation
//>==---------------------------------------------------------------------------

StringLiteral* StringLiteral::create(Rib& rib, SourceSpan span, 
                                     const std::string& value) {
    return new StringLiteral(
        span, 
        PointerType::get(rib, BuiltinType::get(rib, BuiltinType::Kind::Char)),
        value
    );
}

//>==---------------------------------------------------------------------------
//                          BinaryOp Implementation
//>==---------------------------------------------------------------------------

BinaryOp* BinaryOp::create(Rib& rib, SourceSpan span, Operator op, Expr* lhs, 
                           Expr* rhs) {
    assert(op != Unknown && "invalid operator!");
    assert(lhs && "lhs cannot be null!");
    assert(rhs && "rhs cannot be null!");
    return new BinaryOp(span, lhs->type(), op, lhs, rhs);
}

BinaryOp::~BinaryOp() {
    if (m_lhs)
        delete m_lhs;
    
    if (m_rhs)
        delete m_rhs;

    m_lhs = nullptr;
    m_rhs = nullptr;
}

//>==---------------------------------------------------------------------------
//                          UnaryOp Implementation
//>==---------------------------------------------------------------------------

UnaryOp* UnaryOp::create(Rib& rib, SourceSpan span, Operator op, bool prefix, 
                         Expr* expr) {
    assert(op != Unknown && "invalid operator!");
    assert(expr && "expr cannot be null!");
    return new UnaryOp(span, expr->type(), op, prefix, expr);
}

UnaryOp::~UnaryOp() {
    if (m_expr)
        delete m_expr;
    
    m_expr = nullptr;
}

//>==---------------------------------------------------------------------------
//                          AccessExpr Implementation
//>==---------------------------------------------------------------------------

AccessExpr* AccessExpr::create(Rib& rib, SourceSpan span, Expr* base, 
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

AccessExpr::~AccessExpr() {
    if (m_base)
        delete m_base;
    
    m_base = nullptr;
}

//>==---------------------------------------------------------------------------
//                          CallExpr Implementation
//>==---------------------------------------------------------------------------

CallExpr* CallExpr::create(Rib& rib, SourceSpan span, Expr* callee, 
                           const Args& args) {
    assert(callee && "callee cannot be null!");
    return new CallExpr(
        span, 
        callee->type(), 
        callee, 
        args,
        nullptr
    );
}

CallExpr::~CallExpr() {
    if (m_callee)
        delete m_callee;
    
    m_callee = nullptr;
    m_receiver = nullptr;
    
    for (Expr* arg : m_args) {
        if (arg)
            delete arg;
    }

    m_args.clear();
}

//>==---------------------------------------------------------------------------
//                          CastExpr Implementation
//>==---------------------------------------------------------------------------

CastExpr* CastExpr::create(Rib& rib, SourceSpan span, Type* type, 
                           Expr* expr) {
    assert(expr && "expr cannot be null!");
    return new CastExpr(span, type, expr);
}

CastExpr::~CastExpr() {
    if (m_expr)
        delete m_expr;
    
    m_expr = nullptr;
}

//>==---------------------------------------------------------------------------
//                          ParenExpr Implementation
//>==---------------------------------------------------------------------------

ParenExpr* ParenExpr::create(Rib& rib, SourceSpan span, Expr* expr) {
    assert(expr && "expr cannot be null!");
    return new ParenExpr(span, expr->type(), expr);
}

ParenExpr::~ParenExpr() {
    if (m_expr)
        delete m_expr;
    
    m_expr = nullptr;
}

//>==---------------------------------------------------------------------------
//                          RefExpr Implementation
//>==---------------------------------------------------------------------------

RefExpr* RefExpr::create(Rib& rib, SourceSpan span, const std::string& name, 
                         const std::string& spec, ValueDefn* defn) {
    return new RefExpr(span, defn ? defn->type() : nullptr, name, spec, defn);
}

bool RefExpr::is_lvalue() const {
    assert(is_resolved() && "reference not resolved yet!");

    return dynamic_cast<const VariableDefn*>(m_defn) != nullptr
        || dynamic_cast<const ParameterDefn*>(m_defn) != nullptr;
}

//>==---------------------------------------------------------------------------
//                          SizeofExpr Implementation
//>==---------------------------------------------------------------------------

SizeofExpr* SizeofExpr::create(Rib& rib, SourceSpan span, Type* target) {
    return new SizeofExpr(
        span, 
        BuiltinType::get(rib, BuiltinType::Kind::UInt64), 
        target
    );
}

//>==---------------------------------------------------------------------------
//                          FieldInitExpr Implementation
//>==---------------------------------------------------------------------------

FieldInitExpr* FieldInitExpr::create(Rib& rib, SourceSpan span, Type* type, 
                                     const std::string& name, Expr* expr) {
    return new FieldInitExpr(span, type, name, expr);
}

FieldInitExpr::~FieldInitExpr() {
    if (m_expr)
        delete m_expr;

    m_field = nullptr;
    m_expr = nullptr;
}

//>==---------------------------------------------------------------------------
//                          StructInitExpr Implementation
//>==---------------------------------------------------------------------------

StructInitExpr* StructInitExpr::create(Rib& rib, SourceSpan span, 
                                       Type* type, const Fields& fields) {
    return new StructInitExpr(span, type, fields);
}

StructInitExpr::~StructInitExpr() {
    for (FieldInitExpr* fi : m_fields) {
        if (fi)
            delete fi;
    }

    m_fields.clear();
}

//>==---------------------------------------------------------------------------
//                          SubscriptExpr Implementation
//>==---------------------------------------------------------------------------

SubscriptExpr* SubscriptExpr::create(Rib& rib, SourceSpan span, Expr* base, 
                                     Expr* index) {
    assert(base && "base cannot be null!");
    assert(index && "index cannot be null!");
    return new SubscriptExpr(span, base->type(), base, index);
}

SubscriptExpr::~SubscriptExpr() {
    if (m_base)
        delete m_base;
    
    m_base = nullptr;

    if (m_index)
        delete m_index;
    
    m_index = nullptr;
}
