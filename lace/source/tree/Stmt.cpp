//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/tree/Rib.h"
#include "lace/tree/Defn.h"
#include "lace/tree/Expr.h"
#include "lace/tree/VisitorBase.h"
#include "lace/tree/Stmt.h"

using namespace lace;

//>==---------------------------------------------------------------------------
//                          AdapterStmt Implementation
//>==---------------------------------------------------------------------------

AdapterStmt* AdapterStmt::create(Rib& rib, Defn* defn) {
    return new AdapterStmt(defn->span(), defn);
}

AdapterStmt* AdapterStmt::create(Rib& rib, Expr* expr) {
    return new AdapterStmt(expr->span(), expr);
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

BlockStmt* BlockStmt::create(Rib& rib, SourceSpan span, Scope* scope, 
                             const Stmts& stmts) {
    return new BlockStmt(span, scope, stmts);
}

BlockStmt::~BlockStmt() {
    for (Stmt* stmt : m_stmts) {
        if (stmt)
            delete stmt;
    }

    m_stmts.clear();
}

//>==---------------------------------------------------------------------------
//                          IfStmt Implementation
//>==---------------------------------------------------------------------------

IfStmt* IfStmt::create(Rib& rib, SourceSpan span, Expr* cond, Stmt* then, 
                       Stmt* els) {
    return new IfStmt(span, cond, then, els);
}

IfStmt::~IfStmt() {
    if (m_cond)
        delete m_cond;
    
    m_cond = nullptr;

    if (m_then)
        delete m_then;
    
    m_then = nullptr;

    if (m_else)
        delete m_else;

    m_else = nullptr;
}

//>==---------------------------------------------------------------------------
//                          RestartStmt Implementation
//>==---------------------------------------------------------------------------

RestartStmt* RestartStmt::create(Rib& rib, SourceSpan span) {
    return new RestartStmt(span);
}

//>==---------------------------------------------------------------------------
//                          RetStmt Implementation
//>==---------------------------------------------------------------------------

RetStmt* RetStmt::create(Rib& rib, SourceSpan span, Expr* expr) {
    return new RetStmt(span, expr);
}

RetStmt::~RetStmt() {
    if (m_expr)
        delete m_expr;

    m_expr = nullptr;
}

//>==---------------------------------------------------------------------------
//                          StopStmt Implementation
//>==--------------------------------------------------------------------------

StopStmt* StopStmt::create(Rib& rib, SourceSpan span) {
    return new StopStmt(span);
}

//>==---------------------------------------------------------------------------
//                          UntilStmt Implementation
//>==---------------------------------------------------------------------------

UntilStmt* UntilStmt::create(Rib& rib, SourceSpan span, Expr* cond, Stmt* body) {
    return new UntilStmt(span, cond, body);
}

UntilStmt::~UntilStmt() {
    if (m_cond)
        delete m_cond;
    
    m_cond = nullptr;

    if (m_body)
        delete m_body;

    m_body = nullptr;
}

//>==---------------------------------------------------------------------------
//                          RuneStmt Implementation
//>==---------------------------------------------------------------------------

RuneStmt* RuneStmt::create(Rib& rib, SourceSpan span, Rune* rune) {
    return new RuneStmt(span, rune);
}

RuneStmt::~RuneStmt() {
    delete m_rune;
    m_rune = nullptr;
}
