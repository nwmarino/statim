//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/core/Diagnostics.h"
#include "lace/parser/Parser.h"
#include "lace/tree/Defn.h"
#include "lace/tree/Scope.h"
#include "lace/tree/Stmt.h"

#include <cassert>

using namespace lace;

Stmt* Parser::parse_initial_statement() {
    if (match(Token::OpenBrace)) {
        return parse_block_statement();
    } else if (match(Token::Sign)) {
        return parse_rune_statement();
    } else if (match("let")) {
        return parse_declarative_statement();
    } else {
        return parse_control_statement();
    }
}

Stmt* Parser::parse_block_statement() {
    SourceLocation start = loc();
    next(); // '{'

    BlockStmt::Stmts stmts = {};
    stmts.reserve(4);

    Scope* scope = enter_scope();

    while (!match(Token::CloseBrace)) {
        Stmt* stmt = parse_initial_statement();
        if (!stmt)
            log::fatal("expected statement", log::Span(m_file, since(start)));

        while (expect(Token::Semi));
        stmts.push_back(stmt);
    }

    stmts.shrink_to_fit();
    exit_scope();

    SourceLocation end = loc();
    next(); // '}'

    return BlockStmt::create(*m_context, SourceSpan(start, end), scope, stmts);
}

Stmt* Parser::parse_control_statement() {
    const Token ctrl = curr();
    
    if (expect("stop")) {
        return StopStmt::create(*m_context, since(ctrl.loc));
    } else if (expect("restart")) {
        return RestartStmt::create(*m_context, since(ctrl.loc));
    } else if (expect("ret")) {
        Expr* expr = nullptr;
        if (!expect(Token::Semi)) {
            expr = parse_initial_expression();
            if (!expect(Token::Semi))
                log::fatal("expected ';'", log::Span(m_file, since(loc())));
        }

        return RetStmt::create(*m_context, since(ctrl.loc), expr);
    } else if (expect("if")) {
        Expr* cond = parse_initial_expression();
        assert(cond && "unable to parse 'if' condition!");

        Stmt* then_body = parse_initial_statement();
        assert(then_body && "unable to parse 'if' then body!");

        Stmt* else_body = nullptr;
        if (expect("else")) {
            else_body = parse_initial_statement();
            assert(else_body && "unable to parse 'if' else body!");
        }

        return IfStmt::create(
            *m_context, since(ctrl.loc), cond, then_body, else_body);
    } else if (expect("until")) {
        Expr* cond = parse_initial_expression();
        if (!cond)
            log::fatal("expected 'until' condition", log::Span(m_file, since(loc())));

        Stmt* body = nullptr;
        if (!match(Token::Semi)) {
            body = parse_initial_statement();
            if (!body)
                log::fatal("expected 'until' body", log::Span(m_file, since(loc())));
        }

        return UntilStmt::create(*m_context, since(ctrl.loc), cond, body);
    } else {
        Expr* expr = parse_initial_expression();
        if (!expr)
            log::fatal("expected statement", log::Span(m_file, since(loc())));

        return AdapterStmt::create(*m_context, expr);
    }
}

Stmt* Parser::parse_declarative_statement() {
    const SourceLocation start = loc();
    next(); // 'let'

    if (!match(Token::Identifier))
        log::fatal("expected identifier", log::Span(m_file, since(loc())));

    const std::string name = curr().value;
    next();

    if (!expect(Token::Colon))
        log::fatal("expected ':'", log::Span(m_file, since(loc())));

    Type* type = parse_type_specifier();

    SourceLocation end = loc();
    Expr* init = nullptr;
    if (!expect(Token::Semi)) {
        if (!expect(Token::Eq))
            log::fatal("expected '='", log::Span(m_file, since(loc())));

        init = parse_initial_expression();
        end = loc();
        if (!expect(Token::Semi))
            log::fatal("expected ';'", log::Span(m_file, since(loc())));
    }

    VariableDefn* var = VariableDefn::create(
        *m_context, 
        SourceSpan(start, end), 
        name, 
        {}, // runes
        type, 
        init,
        false);

    m_scope->add(var);
    return AdapterStmt::create(*m_context, var);
}

Stmt* Parser::parse_rune_statement() {
    const SourceLocation start = loc();
    next(); // '$'

    static std::unordered_map<std::string, Rune::Kind> runes = {
        { "abort", Rune::Kind::Abort},
        { "unreachable", Rune::Kind::Unreachable },
    };

    if (!match(Token::Identifier))
        log::fatal("expected identifier after '$'", log::Span(m_file, loc()));

    if (!runes.contains(curr().value))
        log::fatal("unknown rune: " + curr().value, log::Span(m_file, loc()));
    
    const SourceLocation end = loc();
    Rune::Kind kind = runes[curr().value];
    next();

    return RuneStmt::create(*m_context, SourceSpan(start, end), new Rune(kind));
}
