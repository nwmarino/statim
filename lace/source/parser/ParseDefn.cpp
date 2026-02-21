//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/core/Diagnostics.h"
#include "lace/parser/Parser.h"
#include "lace/tree/Defn.h"
#include "lace/tree/Type.h"

using namespace lace;

Defn* Parser::parse_initial_definition() {
    std::vector<Rune*> runes = {};
    parse_rune_decorators(runes);

    if (!match(Token::Identifier))
        log::fatal("expected identifier", log::Location(m_file, loc()));

    if (match("load"))
        return parse_load_definition();

    const Token name = curr();
    next();

    if (expect(Token::Path))
        return parse_binding_definition(runes, name);

    return nullptr;
}

Defn* Parser::parse_binding_definition(std::vector<Rune*> runes, const Token name) {
    if (expect(Token::OpenParen)) {
        Scope* scope = enter_scope();

        std::vector<ParameterDefn*> params = {};
        params.reserve(2);

        while (!expect(Token::CloseParen)) {
            const SourceLocation dbg_start = loc();

            if (!match(Token::Identifier))
                log::fatal("expected parameter name", 
                    log::Span(m_file, since(dbg_start)));

            const SourceLocation param_start = loc();
            std::string param_name = curr().value;
            next();

            if (!expect(Token::Colon)) {
                log::fatal("expected ':' after parameter name", 
                    log::Span(m_file, since(dbg_start)));
            }

            Type* param_type = parse_type_specifier();

            ParameterDefn* param = ParameterDefn::create(
                *m_context, since(param_start), param_name, {}, param_type);

            if (param_name != "_")
                m_scope->add(param);
            
            params.push_back(param);

            if (expect(Token::CloseParen))
                break;

            if (!expect(Token::Comma))
                log::fatal("expected ','", log::Span(m_file, since(dbg_start)));
        }

        params.shrink_to_fit();

        if (!expect(Token::Arrow)) {
            log::fatal("expected '->' after parameter list", 
                log::Span(m_file, since(name.loc)));
        }

        Type* ret_type = parse_type_specifier();

        BlockStmt* body = nullptr;
        SourceLocation end = loc();
        if (match(Token::OpenBrace)) {
            body = static_cast<BlockStmt*>(parse_block_statement());
            end = body->get_span().end;
        } else if (!expect(Token::Semi)) {
            log::fatal("expected function body", 
                log::Span(m_file, since(name.loc)));
        }

        exit_scope();

        std::vector<Type*> param_types = {};
        param_types.reserve(params.size());
        for (ParameterDefn* param : params)
            param_types.push_back(param->type());

        FunctionDefn* defn = FunctionDefn::create(
            *m_context, 
            SourceSpan(name.loc, end), 
            name.value,
            runes,
            FunctionType::get(*m_context, ret_type, param_types), 
            scope, 
            params, 
            body);

        m_scope->add(defn);
        return defn;
    } else if (expect("struct")) {
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
                *m_context, 
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
            *m_context, 
            SourceSpan(name.loc, end), 
            name.value, 
            runes,
            nullptr
        );

        StructType* type = StructType::create(*m_context, defn);
        
        defn->set_type(type);
        defn->set_fields(fields);
        m_scope->add(defn);
        return defn;
    } else if (expect("enum")) {
        Type* underlying;
        if (match(Token::Identifier)) {
            underlying = parse_type_specifier();
        } else {
            underlying = BuiltinType::get(*m_context, BuiltinType::Kind::Int64);
        }

        EnumDefn* defn = EnumDefn::create(
            *m_context, 
            name.loc, 
            name.value, 
            runes,
            underlying
        );

        EnumType* type = EnumType::create(*m_context, underlying, defn);
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
                *m_context, 
                since(variant_name.loc), 
                variant_name.value, 
                {},
                type, 
                value++
            );

            m_scope->add(variant);
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
        m_scope->add(defn);
        return defn;
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
            *m_context, 
            SourceSpan(name.loc, end), 
            name.value, 
            runes,
            type, 
            init, 
            true);

        m_scope->add(var);
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
        *m_context, 
        SourceSpan { start, path.loc }, 
        path.value
    );
}
