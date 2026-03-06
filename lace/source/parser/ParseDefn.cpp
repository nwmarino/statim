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

    if (match("use"))
        return parse_use();

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

    log::Span lspan = { m_rib->path(), function_id.loc };

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
            *m_rib, 
            SourceSpan { receiver_id.loc, loc() }, 
            receiver_id.value,
            {}, 
            receiver_type
        );

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
            *m_rib, 
            SourceSpan { param_id.loc, loc() },
            param_id.value,
            {},
            param_type
        );
        
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

    std::vector<Type*> param_types = {};
    param_types.reserve(params.size());

    if (receiver)
        param_types.push_back(receiver->type());

    for (ParameterDefn* param : params)
        param_types.push_back(param->type());

    FunctionDefn* defn = FunctionDefn::create(
        *m_rib, 
        SourceSpan { function_id.loc, loc() },
        function_id.value,
        runes,
        FunctionType::get(*m_rib, result_type, param_types), 
        new Scope(), 
        receiver,
        params, 
        body
    );
    
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
            Expr* init = nullptr;

            if (expect(Token::Eq)) {
                init = parse_initial_expression();
                if (!init)
                    log::fatal("expected expression after '='", log::Span(m_file, dbg_start));    
            }

            FieldDefn* field = FieldDefn::create(
                *m_rib, 
                since(field_name.loc), 
                field_name.value, 
                {},
                field_type,
                init,
                fields.size()
            );

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
            *m_rib, 
            SourceSpan(name.loc, end), 
            name.value, 
            runes,
            nullptr
        );

        defn->set_type(StructType::create(*m_rib, defn));
        defn->set_fields(fields);
        return defn;
    } else if (expect("enum")) {
        Type* underlying;
        if (match(Token::Identifier)) {
            underlying = parse_type_specifier();
        } else {
            underlying = BuiltinType::get(*m_rib, BuiltinType::Kind::Int64);
        }

        EnumDefn* defn = EnumDefn::create(
            *m_rib, 
            name.loc, 
            name.value, 
            runes,
            underlying
        );

        EnumType* type = EnumType::create(*m_rib, underlying, defn);
        defn->set_type(type);
        
        if (!expect(Token::OpenBrace))
            log::fatal("expected '{'", log::Span(m_file, since(name.loc)));

        std::vector<VariantDefn*> variants = {};

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

            variants.push_back(VariantDefn::create(
                *m_rib, 
                since(variant_name.loc), 
                variant_name.value, 
                {},
                type, 
                value++
            ));

            if (match(Token::CloseBrace)) {
                end = loc();
                next(); // '}'
                break;
            }

            if (!expect(Token::Comma))
                log::fatal("expected ','", log::Span(m_file, since(loc())));
        }

        defn->set_variants(variants);
        return defn;
    } else {
        // Assume global variable definition.
        Type* type = parse_type_specifier();

        Expr* init = nullptr;
        SourceLocation end = loc();
        
        if (expect(Token::Eq)) {
            init = parse_initial_expression();
            end = init->span().end;
        }

        // Semis are not strictly necessary, but are not disallowed either.
        while (expect(Token::Semi));

        return VariableDefn::create(
            *m_rib, 
            SourceSpan(name.loc, end), 
            name.value, 
            runes,
            type, 
            init, 
            true
        );
    }

    return nullptr;
}

Defn* Parser::parse_use() {
    assert(curr().value == "use");

    const SourceLocation start = loc();
    next(); // 'use'

    return UseDefn::create(*m_rib, since(start), parse_trail());
}
