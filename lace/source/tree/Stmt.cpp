//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lace/tree/AST.hpp"
#include "lace/tree/Defn.hpp"
#include "lace/tree/Expr.hpp"
#include "lace/tree/Scope.hpp"
#include "lace/tree/VisitorBase.hpp"
#include "lace/tree/Stmt.hpp"

using namespace lace;

/*
AsmStmt::~AsmStmt() {
    for (Expr* arg : m_args)
        delete arg;

    m_ins.clear();
    m_outs.clear();
    m_args.clear();
    m_clobbers.clear();
}

AsmStmt* AsmStmt::create(
        Context &ctx, SourceSpan span, const string& iasm, 
        const vector<string> &outs, const vector<string> &ins, 
        const vector<Expr*> &args, const vector<string> &clobbers) {
    return new AsmStmt(span, iasm, outs, ins, args, clobbers);
}
*/

AdapterStmt* AdapterStmt::create(AST::Context& ctx, Defn* defn) {
    return new AdapterStmt(defn->get_span(), defn);
}

AdapterStmt* AdapterStmt::create(AST::Context& ctx, Expr* expr) {
    return new AdapterStmt(expr->get_span(), expr);
}

AdapterStmt::~AdapterStmt() {
    switch (m_flavor) {
        case Definitive:
            delete m_defn;
            m_defn = nullptr;
            break;
        case Expressive:
            delete m_expr;
            m_expr = nullptr;
            break;
    }
}

BlockStmt::~BlockStmt() {
    delete m_scope;
    m_scope = nullptr;

    for (Stmt* stmt : m_stmts)
        delete stmt;

    m_stmts.clear();
}

BlockStmt* BlockStmt::create(AST::Context& ctx, SourceSpan span, Scope* scope, 
                             const Stmts& stmts) {
    return new BlockStmt(span, scope, stmts);
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

IfStmt* IfStmt::create(AST::Context& ctx, SourceSpan span, Expr* cond, 
                       Stmt* then, Stmt* els) {
    return new IfStmt(span, cond, then, els);
}

RestartStmt* RestartStmt::create(AST::Context& ctx, SourceSpan span) {
    return new RestartStmt(span);
}

RetStmt::~RetStmt() {
    if (has_expr()) {
        delete m_expr;
        m_expr = nullptr;
    }
}

RetStmt* RetStmt::create(AST::Context& ctx, SourceSpan span, Expr* expr) {
    return new RetStmt(span, expr);
}

StopStmt* StopStmt::create(AST::Context& ctx, SourceSpan span) {
    return new StopStmt(span);
}

UntilStmt::~UntilStmt() {
    delete m_cond;
    m_cond = nullptr;

    if (has_body()) {
        delete m_body;
        m_body = nullptr;
    }
}

UntilStmt* UntilStmt::create(AST::Context& ctx, SourceSpan span, Expr* cond, 
                             Stmt* body) {
    return new UntilStmt(span, cond, body);
}

RuneStmt* RuneStmt::create(AST::Context& ctx, SourceSpan span, Rune* rune) {
    return new RuneStmt(span, rune);
}

RuneStmt::~RuneStmt() {
    delete m_rune;
    m_rune = nullptr;
}
