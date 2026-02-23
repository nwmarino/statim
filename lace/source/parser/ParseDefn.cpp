//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/core/Diagnostics.h"
#include "lace/parser/Parser.h"
#include "lace/tree/Defn.h"
#include "lace/tree/Type.h"
#include "lace/types/SourceLocation.h"

#include <vector>

using namespace lace;

Defn* Parser::parse_initial_definition() {
    std::vector<Rune*> runes = {};
    parse_rune_decorators(runes);

    if (!match(Token::Identifier))
        log::fatal("expected identifier", log::Location(m_file, loc()));

    if (match("load"))
        return parse_load_definition();

    uint64_t position = m_stream.position();
    next(); // id

    if (match(Token::OpenParen)) {
        // This is a function with a receiver.
        return parse_function_definition(runes, position);
    } else if (match(Token::Path)) {
        next(); // '::'

        if (match(Token::OpenParen)) {
            // This is a function.
            return parse_function_definition(runes, position);
        } else {
            return parse_binding_definition(runes, m_stream.get(position));
        }
    }

    return nullptr;
}

Defn* Parser::parse_function_definition(std::vector<Rune*> runes, uint64_t start) {
    m_stream.seek(start);

    assert(match(Token::Identifier));

    const Token function_id = curr();
    next(); // id

    Scope* scope = enter_scope();

    log::Span lspan = { m_ast->get_file(), function_id.loc };

    // Parse a receiver, if the function has one.
    ParameterDefn* receiver = nullptr;
    if (expect(Token::OpenParen)) {
        if (!match(Token::Identifier))
            log::fatal("expected receiver name", lspan);

        const Token receiver_id = curr();
        next(); // id

        if (!expect(Token::Colon))
            log::fatal("expected ':' after receiver name", lspan);

        Type* receiver_type = parse_type_specifier();
        assert(receiver_type);

        if (!dynamic_cast<PointerType*>(receiver_type))
            log::fatal("receiver type must be a pointer", lspan);

        receiver = ParameterDefn::create(
            *m_ast, 
            SourceSpan { receiver_id.loc, loc() }, 
            receiver_id.value,
            {}, 
            receiver_type
        );

        if (!m_scope->add(receiver))
            log::fatal("name already exists: " + receiver->name(), lspan);

        if (!expect(Token::CloseParen))
            log::fatal("expected ')' after function receiver", lspan);
    }

    if (!expect(Token::Path))
        log::fatal("expected '::'", lspan);

    if (!expect(Token::OpenParen))
        log::fatal("expected '(' after '::'", lspan);

    std::vector<ParameterDefn*> params = {};
    while (!expect(Token::CloseParen)) {
        if (!match(Token::Identifier))
            log::fatal("expected parameter name", lspan);

        const Token param_id = curr();
        next(); // id

        if (!expect(Token::Colon))
            log::fatal("expected ':' after parameter name", lspan);

        Type* param_type = parse_type_specifier();
        assert(param_type);

        ParameterDefn* param = ParameterDefn::create(
            *m_ast, 
            SourceSpan { param_id.loc, loc() },
            param_id.value,
            {},
            param_type
        );

        if (param->name() != "_") {
            // If the parameter name isn't the default '_', then add it to the
            // function scope.
            if (!m_scope->add(param))
                log::fatal("name already exists: " + param->name(), lspan);
        }
        
        params.push_back(param);

        if (expect(Token::CloseParen))
            break;

        if (!expect(Token::Comma))
            log::fatal("expected ',' or ')' after parameter", lspan);
    }

    if (!expect(Token::Arrow))
        log::fatal("expected '->' after parameter list", lspan);

    Type* result_type = parse_type_specifier();
    assert(result_type);

    BlockStmt* body = nullptr;
    if (match(Token::OpenBrace)) {
        Stmt* stmt = parse_block_statement();
        if (!stmt)
            log::fatal("expected function body", lspan);

        body = dynamic_cast<BlockStmt*>(stmt);
        assert(body);
    } else if (!expect(Token::Semi)) {
        log::fatal("expected ';' or '{'", lspan);
    }

    exit_scope();

    std::vector<Type*> param_types = {};
    param_types.reserve(params.size());

    if (receiver)
        param_types.push_back(receiver->type());

    for (ParameterDefn* param : params)
        param_types.push_back(param->type());

    FunctionDefn* defn = FunctionDefn::create(
        *m_ast, 
        SourceSpan { function_id.loc, loc() },
        function_id.value,
        runes,
        FunctionType::get(*m_ast, result_type, param_types), 
        scope, 
        receiver,
        params, 
        body
    );

    if (!m_scope->add(defn))
        log::fatal("name already exists: " + defn->name(), lspan);
    
    return defn;
}

Defn* Parser::parse_binding_definition(std::vector<Rune*> runes, const Token name) {
    if (expect("struct")) {
        if (!expect(Token::OpenBrace))
            log::fatal("expected '{'", log::Span(m_file, since(loc())));

        std::vector<FieldDefn*> fields = {};
        fields.reserve(2);

        SourceLocation end = loc();
        while (!expect(Token::CloseBrace)) {
            const SourceLocation dbg_start = loc();

            if (!match(Token::Identifier))
                log::fatal("expected field name", log::Span(m_file, dbg_start));

            const Token field_name = curr();
            next();

            if (!expect(Token::Colon))
                log::fatal("expected ':'", log::Span(m_file, dbg_start));

            Type* field_type = parse_type_specifier();

            FieldDefn* field = FieldDefn::create(
                *m_ast, 
                since(field_name.loc), 
                field_name.value, 
                {},
                field_type,
                fields.size());

            fields.push_back(field);

            if (match(Token::CloseBrace)) {
                end = loc();
                next(); // '}'
                break;
            }

            if (!expect(Token::Comma))
                log::fatal("expected ','", log::Span(m_file, since(name.loc)));
        }

        fields.shrink_to_fit();
        
        StructDefn* defn = StructDefn::create(
            *m_ast, 
            SourceSpan(name.loc, end), 
            name.value, 
            runes,
            nullptr
        );

        StructType* type = StructType::create(*m_ast, defn);
        
        defn->set_type(type);
        defn->set_fields(fields);

        if (!m_scope->add(defn)) {
            log::fatal("name already exists in scope: " + defn->name(),
                log::Span(m_file, since(name.loc)));
        }

        return defn;
    } else if (expect("enum")) {
        Type* underlying;
        if (match(Token::Identifier)) {
            underlying = parse_type_specifier();
        } else {
            underlying = BuiltinType::get(*m_ast, BuiltinType::Kind::Int64);
        }

        EnumDefn* defn = EnumDefn::create(
            *m_ast, 
            name.loc, 
            name.value, 
            runes,
            underlying
        );

        EnumType* type = EnumType::create(*m_ast, underlying, defn);
        defn->set_type(type);
        
        if (!expect(Token::OpenBrace))
            log::fatal("expected '{'", log::Span(m_file, since(name.loc)));

        std::vector<VariantDefn*> variants = {};
        variants.reserve(4);

        int64_t value = 0;
        SourceLocation end = loc();
        while (!expect(Token::CloseBrace)) {
            if (!match(Token::Identifier))
                log::fatal("expected name", log::Span(m_file, since(name.loc)));

            const Token variant_name = curr();
            next();

            if (expect(Token::Eq)) {
                const SourceLocation dbg_start = loc();
                bool neg = false;

                if (expect(Token::Minus))
                    neg = true;

                if (!match(Token::Integer)) {
                    log::fatal("expected integer", 
                        log::Span(m_file, since(dbg_start)));
                }

                value = std::stoll(curr().value);
                if (neg)
                    value = -value;

                next();
            }

            VariantDefn* variant = VariantDefn::create(
                *m_ast, 
                since(variant_name.loc), 
                variant_name.value, 
                {},
                type, 
                value++
            );

            if (!m_scope->add(variant)) {
                log::fatal("name already exists in scope: " + variant->name(), 
                    log::Span(m_file, since(loc())));
            }
            
            variants.push_back(variant);

            if (match(Token::CloseBrace)) {
                end = loc();
                next(); // '}'
                break;
            }

            if (!expect(Token::Comma))
                log::fatal("expected ','", log::Span(m_file, since(loc())));
        }

        variants.shrink_to_fit();

        defn->set_variants(variants);

        if (!m_scope->add(defn)) {
            log::fatal("name already exists in scope: " + defn->name(),
                log::Span(m_file, since(loc())));
        }

        return defn;
    } else if (expect("space")) {
        if (!expect(Token::OpenBrace))
            log::fatal("expected '{'", log::Span(m_file, since(loc())));
        
        SpaceDefn* existing = nullptr;
        if (NamedDefn* defn = m_scope->get(name.value)) {
            existing = dynamic_cast<SpaceDefn*>(defn);
            if (existing) {
                // @Todo: PROVE this is always true, or change it.
                assert(existing->scope()->parent() == m_scope);

                m_scope = existing->scope();
            } else {
                enter_scope();
            }
        } else {
            enter_scope();
        }

        std::vector<NamedDefn*> defns = {};
        while (!match(Token::CloseBrace)) {
            Defn* defn = parse_initial_definition();
            if (!defn)
                log::fatal("expected definition", log::Span(m_file, since(loc())));
            
            NamedDefn* named = dynamic_cast<NamedDefn*>(defn);
            if (!named)
                log::fatal("expected named definition", log::Span(m_file, since(loc())));

            defns.push_back(named);
        }

        SpaceDefn* space = SpaceDefn::create(
            *m_ast, 
            SourceSpan { name.loc, loc() }, 
            name.value, 
            runes, 
            m_scope, 
            defns
        );

        exit_scope();

        next(); // '}'

        if (existing)
            return space;

        if (!m_scope->add(space)) {
            log::fatal("name already exists in scope: " + space->name(), 
                log::Span(m_file, space->span()));
        }

        return space;
    } else {
        // Assume global variable definition.
        Type* type = parse_type_specifier();

        Expr* init = nullptr;
        SourceLocation end = loc();
        
        if (expect(Token::Eq)) {
            init = parse_initial_expression();
            end = init->get_span().end;
        }

        // Semis are not strictly necessary, but are not disallowed either.
        while (expect(Token::Semi));

        VariableDefn* var = VariableDefn::create(
            *m_ast, 
            SourceSpan(name.loc, end), 
            name.value, 
            runes,
            type, 
            init, 
            true
        );

        if (!m_scope->add(var)) {
            log::fatal("name already exists in scope: " + var->name(), 
                log::Span(m_file, since(name.loc)));
        }

        return var;
    }

    return nullptr;
}

Defn* Parser::parse_load_definition() {
    const SourceLocation start = loc();
    next(); // 'load'

    if (!match(Token::String))
        log::fatal("expected file path", log::Span(m_file, since(start)));

    const Token path = curr();
    next();

    while (expect(Token::Semi));

    return LoadDefn::create(
        *m_ast, 
        SourceSpan { start, path.loc }, 
        path.value
    );
}
