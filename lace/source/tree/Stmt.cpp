//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/tree/AST.h"
#include "lace/tree/Defn.h"
#include "lace/tree/Expr.h"
#include "lace/tree/VisitorBase.h"
#include "lace/tree/Stmt.h"

using namespace lace;

//>==---------------------------------------------------------------------------
//                          AdapterStmt Implementation
//>==---------------------------------------------------------------------------

AdapterStmt* AdapterStmt::create(AST::Context& ctx, Defn* defn) {
    return new AdapterStmt(defn->get_span(), defn);
}

AdapterStmt* AdapterStmt::create(AST::Context& ctx, Expr* expr) {
    return new AdapterStmt(expr->get_span(), expr);
}

AdapterStmt::~AdapterStmt() {
    switch (m_kind) 
    {
    case Kind::Definitive:
        if (m_defn)
            delete m_defn;

        m_defn = nullptr;
        break;
    case Kind::Expressive:
        if (m_expr)
            delete m_expr;
        
        m_expr = nullptr;
        break;
    }
}

//>==---------------------------------------------------------------------------
//                          BlockStmt Implementation
//>==---------------------------------------------------------------------------

BlockStmt* BlockStmt::create(AST::Context& ctx, SourceSpan span, Scope* scope, 
                             const Stmts& stmts) {
    return new BlockStmt(span, scope, stmts);
}

BlockStmt::~BlockStmt() {
    delete m_scope;
    m_scope = nullptr;

    for (Stmt* stmt : m_stmts)
        delete stmt;

    m_stmts.clear();
}

//>==---------------------------------------------------------------------------
//                          IfStmt Implementation
//>==---------------------------------------------------------------------------

IfStmt* IfStmt::create(AST::Context& ctx, SourceSpan span, Expr* cond, 
                       Stmt* then, Stmt* els) {
    return new IfStmt(span, cond, then, els);
}

IfStmt::~IfStmt() {
    delete m_cond;
    m_cond = nullptr;

    delete m_then;
    m_then = nullptr;

    if (has_else()) {
        delete m_else;
        m_else = nullptr;
    }
}

//>==---------------------------------------------------------------------------
//                          RestartStmt Implementation
//>==---------------------------------------------------------------------------

RestartStmt* RestartStmt::create(AST::Context& ctx, SourceSpan span) {
    return new RestartStmt(span);
}

//>==---------------------------------------------------------------------------
//                          RetStmt Implementation
//>==---------------------------------------------------------------------------

RetStmt* RetStmt::create(AST::Context& ctx, SourceSpan span, Expr* expr) {
    return new RetStmt(span, expr);
}

RetStmt::~RetStmt() {
    if (has_expr()) {
        delete m_expr;
        m_expr = nullptr;
    }
}

//>==---------------------------------------------------------------------------
//                          StopStmt Implementation
//>==--------------------------------------------------------------------------

StopStmt* StopStmt::create(AST::Context& ctx, SourceSpan span) {
    return new StopStmt(span);
}

//>==---------------------------------------------------------------------------
//                          UntilStmt Implementation
//>==---------------------------------------------------------------------------

UntilStmt* UntilStmt::create(AST::Context& ctx, SourceSpan span, Expr* cond, 
                             Stmt* body) {
    return new UntilStmt(span, cond, body);
}

UntilStmt::~UntilStmt() {
    delete m_cond;
    m_cond = nullptr;

    if (has_body()) {
        delete m_body;
        m_body = nullptr;
    }
}

//>==---------------------------------------------------------------------------
//                          RuneStmt Implementation
//>==---------------------------------------------------------------------------

RuneStmt* RuneStmt::create(AST::Context& ctx, SourceSpan span, Rune* rune) {
    return new RuneStmt(span, rune);
}

RuneStmt::~RuneStmt() {
    delete m_rune;
    m_rune = nullptr;
}
