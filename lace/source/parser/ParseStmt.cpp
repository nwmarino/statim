//
//  Copyright (c) 2025-2026 Nick Marino
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
    //} else if (match("asm")) {
    //   return parse_inline_assembly_statement();
    } else if (match("let")) {
        return parse_declarative_statement();
    } else {
        return parse_control_statement();
    }
}

/*
Stmt* Parser::parse_inline_assembly_statement() {
    const SourceLocation start = loc();
    next(); // 'asm'

    if (!expect(Token::SetBrace))
        m_diags.fatal("expected '{'", SourceSpan(loc()));

    string iasm;
    vector<Expr*> args = {};
    vector<string> outs = {};
    vector<string> ins = {};
    vector<string> clobbers = {};

    // Parse the assembly template (between '{' and the first ':').
    while (!expect(Token::Colon)) {
        if (!match(Token::String))
            m_diags.fatal("expected inline assembly string literal", SourceSpan(loc()));

        iasm += last().value;
        if (iasm.back() != '\n')
            iasm.push_back('\n');
        
        next();
    }

    // Parse the output constraints (between the first ':' and the second ':').
    while (!expect(Token::Colon)) {
        if (!match(Token::String))
            m_diags.fatal("expected output constraint string", SourceSpan(loc()));

        outs.push_back(last().value);
        next();

        if (!expect(Token::SetParen))
            m_diags.fatal("expected '('", SourceSpan(loc()));

        Expr* arg = parse_initial_expression();
        if (!arg)
            m_diags.fatal("expected expression", SourceSpan(loc()));

        args.push_back(arg);

        if (!expect(Token::EndParen))
            m_diags.fatal("expected ')'", SourceSpan(loc()));

        if (expect(Token::Colon))
            break;

        if (!expect(Token::Comma))
            m_diags.fatal("expected ','", SourceSpan(loc()));
    }

    // Parse the input constraints (between the second ':' and the third ':').
    while (!expect(Token::Colon)) {
        if (!match(Token::String))
            m_diags.fatal("expected input constraint string", SourceSpan(loc()));

        ins.push_back(last().value);
        next();

        if (!expect(Token::SetParen))
            m_diags.fatal("expected '('", SourceSpan(loc()));

        Expr* arg = parse_initial_expression();
        if (!arg)
            m_diags.fatal("expected expression", SourceSpan(loc()));

        args.push_back(arg);

        if (!expect(Token::EndParen))
            m_diags.fatal("expected ')'", SourceSpan(loc()));

        if (expect(Token::Colon))
            break;

        if (!expect(Token::Comma))
            m_diags.fatal("expected ','", SourceSpan(loc()));
    }

    // Parse the clobbers (between the third ':' and the '}').
    while (!match(Token::EndBrace)) {
        if (!match(Token::String))
            m_diags.fatal("expected clobber string", SourceSpan(loc()));

        clobbers.push_back(last().value);
        next();

        if (match(Token::EndBrace))
            break;

        if (!expect(Token::Comma))
            m_diags.fatal("expected ','", SourceSpan(loc()));
    }

    const SourceLocation end = loc();
    next(); // ')'

    return AsmStmt::create(
        *m_context, SourceSpan(start, end), iasm, outs, ins, args, clobbers);
}
*/

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

    QualType type = parse_type_specifier();

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

    static std::unordered_map<std::string, Rune::Type> runes = {
        { "abort", Rune::Abort},
        { "unreachable", Rune::Unreachable },
    };

    if (!match(Token::Identifier))
        log::fatal("expected identifier after '$'", log::Span(m_file, loc()));

    if (!runes.contains(curr().value))
        log::fatal("unknown rune: " + curr().value, log::Span(m_file, loc()));
    
    const SourceLocation end = loc();
    Rune::Type type = runes[curr().value];
    next();

    return RuneStmt::create(*m_context, SourceSpan(start, end), new Rune(type));
}
