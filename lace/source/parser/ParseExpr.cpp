//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/core/Diagnostics.h"
#include "lace/parser/Parser.h"
#include "lace/tree/Expr.h"
#include "lace/tree/Type.h"
#include "lace/types/SourceLocation.h"

#include <cassert>
#include <string>
#include <vector>

using namespace lace;

UnaryOp::Operator Parser::get_unary_op(Token::Kind kind) const {
    switch (kind) 
    {
    case Token::Bang:       
        return UnaryOp::LogicNot;
    case Token::Minus:      
        return UnaryOp::Negate;
    case Token::Star:       
        return UnaryOp::Dereference;
    case Token::And:        
        return UnaryOp::AddressOf;
    case Token::Tilde:      
        return UnaryOp::Not;
    default:                
        return UnaryOp::Unknown;
    }
}

BinaryOp::Operator Parser::get_binary_op(Token::Kind kind) const {
    switch (kind) 
    {
    case Token::Eq:             
        return BinaryOp::Assign;
    case Token::EqEq:           
        return BinaryOp::Eq;
    case Token::BangEq:         
        return BinaryOp::NEq;
    case Token::Plus:           
        return BinaryOp::Add;
    case Token::Minus:          
        return BinaryOp::Sub;
    case Token::Star:           
        return BinaryOp::Mul;
    case Token::Slash:          
        return BinaryOp::Div;
    case Token::Percent:        
        return BinaryOp::Mod;
    case Token::Left:           
        return BinaryOp::Lt;
    case Token::LeftLeft:       
        return BinaryOp::LShift;
    case Token::LeftEq:         
        return BinaryOp::LtEq;
    case Token::Right:          
        return BinaryOp::Gt;
    case Token::RightRight:     
        return BinaryOp::RShift;
    case Token::RightEq:        
        return BinaryOp::GtEq;
    case Token::And:            
        return BinaryOp::And;
    case Token::AndAnd:         
        return BinaryOp::LogicAnd;
    case Token::Or:             
        return BinaryOp::Or;
    case Token::OrOr:           
        return BinaryOp::LogicOr;
    case Token::Xor:            
        return BinaryOp::Xor;
    default:                    
        return BinaryOp::Unknown;
    }
}

int8_t Parser::get_op_precedence(BinaryOp::Operator op) const {
    switch (op) 
    {
    case BinaryOp::Mul:
    case BinaryOp::Div:
    case BinaryOp::Mod:
        return 11;
    case BinaryOp::Add:
    case BinaryOp::Sub:
        return 10;
    case BinaryOp::LShift:
    case BinaryOp::RShift:
        return 9;
    case BinaryOp::Lt:
    case BinaryOp::LtEq:
    case BinaryOp::Gt:
    case BinaryOp::GtEq:
        return 8;
    case BinaryOp::Eq:
    case BinaryOp::NEq:
        return 7;
    case BinaryOp::And:
        return 6;
    case BinaryOp::Or:
        return 5;
    case BinaryOp::Xor:
        return 4;
    case BinaryOp::LogicAnd:
        return 3;
    case BinaryOp::LogicOr:
        return 2;
    case BinaryOp::Assign:
        return 1;
    case BinaryOp::Unknown:
        return -1;
    }
}

Expr* Parser::parse_initial_expression() {
    const SourceLocation loc_start = loc();
    Expr* expr = parse_prefix_operator();
    if (!expr)
        log::fatal("expected expression", log::Span(m_file, since(loc_start)));

    return parse_binary_operator(expr, 0);
}

Expr* Parser::parse_primary_expression() {
    if (match(Token::Identifier)) {
        return parse_identifier_expression();
    } else if (match(Token::OpenParen)) {
        return parse_parentheses();
    } else if (match(Token::Integer)) {
        return parse_literal_int();
    } else if (match(Token::Float)) {
        return parse_literal_float();
    } else if (match(Token::Character)) {
        return parse_literal_char();
    } else if (match(Token::String)) {
        return parse_literal_string();
    }
     
    return nullptr;
}

Expr* Parser::parse_identifier_expression() {
    if (match("cast")) {
        return parse_type_cast();
    } else if (match("null")) {
        return parse_literal_null();
    } else if (match("true") || match("false")) {
        return parse_literal_bool();
    } else if (match("sizeof")) {
        return parse_sizeof_operator();
    } else {
        return parse_named_reference();
    }
}

Expr* Parser::parse_prefix_operator() {
    UnaryOp::Operator op = get_unary_op(curr().kind);
    if (UnaryOp::is_prefix(op)) {
        const SourceLocation start = loc();
        next();

        Expr* base = parse_prefix_operator();
        if (!base)
            log::fatal("expected expression", log::Span(m_file, since(start)));

        return UnaryOp::create(*m_rib, since(start), op, true, base);
    } else {
        return parse_postfix_operator();
    }
}

Expr* Parser::parse_postfix_operator() {
    const SourceLocation start = loc();
    Expr* expr = parse_primary_expression();
    if (!expr)
        log::fatal("expected expression", log::Span(m_file, since(start)));

    do {
        UnaryOp::Operator op = get_unary_op(curr().kind);

        if (UnaryOp::is_postfix(op)) {
            // Ordinary postfix operator -> UnaryOp recurse.
            next();
            expr = UnaryOp::create(*m_rib, since(start), op, false, expr);
        } else if (match(Token::OpenBrack)) {
            // '[]' operator -> SubscriptExpr.
            next(); // '['

            Expr* index = parse_initial_expression();
            assert(index && "unable to parse subscript index expression!");

            if (!expect(Token::CloseBrack))
                log::fatal("expected ']'", log::Span(m_file, since(start)));

            expr = SubscriptExpr::create(*m_rib, since(start), expr, index);
        } else if (match(Token::OpenParen)) {
            // '(...' operator -> CallExpr.
            next(); // '('

            std::vector<Expr*> args = {};
            args.reserve(2);

            while (!expect(Token::CloseParen)) {
                const SourceLocation dbg_start = loc();
                Expr* arg = parse_initial_expression();
                if (!arg)
                    log::fatal("expected call argument", 
                        log::Span(m_file, since(dbg_start)));

                args.push_back(arg);

                if (expect(Token::CloseParen))
                    break;

                if (!expect(Token::Comma))
                    log::fatal("expected ','", 
                        log::Span(m_file, since(dbg_start)));
            }

            args.shrink_to_fit();

            expr = CallExpr::create(*m_rib, since(start), expr, args);
        } else if (match(Token::Dot)) {
            // '.' operator -> AccessExpr.
            next(); // '.'

            if (!match(Token::Identifier))
                log::fatal("expected identifier", log::Span(m_file, since(start)));
        
            const std::string field = curr().value;
            next();

            expr = AccessExpr::create(*m_rib, since(start), expr, field);
        } else {
            break;
        }
    } while (true);

    return expr;
}

Expr* Parser::parse_binary_operator(Expr* base, int8_t precedence) {
    const SourceLocation start = loc();

    while (true) {
        BinaryOp::Operator op = get_binary_op(curr().kind);
        if (op == BinaryOp::Unknown)
            break;

        int8_t curr_precedence = get_op_precedence(op);
        if (curr_precedence < precedence)
            break;

        next();
        Expr* right = parse_prefix_operator();
        assert(right && "unable to parse rhs expression!");

        int8_t next_precedence = get_op_precedence(get_binary_op(curr().kind));
        if (curr_precedence < next_precedence) {
            right = parse_binary_operator(right, precedence + 1);
            if (!right)
                log::fatal("expected expression", log::Span(m_file, since(start)));
        }

        base = BinaryOp::create(
            *m_rib, 
            since(base->span().start), 
            op, 
            base, 
            right
        );
    };

    return base;
}

Expr* Parser::parse_literal_bool() {
    const Token lit = curr();
    next();

    return BoolLiteral::create(*m_rib, lit.loc, lit.value == "true");
}

Expr* Parser::parse_literal_int() {
    const Token lit = curr();
    next();

    BuiltinType::Kind kind = BuiltinType::Kind::Int64;
    if (expect("b")) {
        kind = BuiltinType::Kind::Int8;
    } else if (expect("ub")) {
        kind = BuiltinType::Kind::UInt8;
    } else if (expect("s")) {
        kind = BuiltinType::Kind::Int16;
    } else if (expect("us")) {
        kind = BuiltinType::Kind::UInt16;
    } else if (expect("i")) {
        kind = BuiltinType::Kind::Int32;
    } else if (expect("ui")) {
        kind = BuiltinType::Kind::UInt32;
    } else if (expect("l")) {
        kind = BuiltinType::Kind::Int64;
    } else if (expect("ul")) {
        kind = BuiltinType::Kind::UInt64;
    }

    return IntegerLiteral::create(
        *m_rib, 
        lit.loc, 
        BuiltinType::get(*m_rib, kind), 
        std::stoll(lit.value));
}

Expr* Parser::parse_literal_float() {
    const Token lit = curr();
    next();

    BuiltinType::Kind kind = BuiltinType::Kind::Float64;
    if (expect("f")) {
        kind = BuiltinType::Kind::Float32;
    } else if (expect("d")) {
        kind = BuiltinType::Kind::Float64;
    }

    return FloatLiteral::create(
        *m_rib, 
        lit.loc, 
        BuiltinType::get(*m_rib, kind), 
        std::stod(lit.value)
    );
}

Expr* Parser::parse_literal_char() {
    const Token lit = curr();
    next();

    return CharLiteral::create(*m_rib, lit.loc, lit.value[0]);
}

Expr* Parser::parse_literal_null() {
    const Token lit = curr();
    next();

    return NullLiteral::create(*m_rib, lit.loc, PointerType::get(
        *m_rib, 
        BuiltinType::get(*m_rib, BuiltinType::Kind::Void)
    ));
}

Expr* Parser::parse_literal_string() {
    const Token lit = curr();
    next();

    return StringLiteral::create(*m_rib, lit.loc, lit.value);
}

Expr* Parser::parse_type_cast() {
    const SourceLocation start = loc();
    next(); // 'cast'

    if (!expect(Token::Left))
        log::fatal("expected '<'", log::Span(m_file, since(start)));

    Type* type = parse_type_specifier();

    if (!expect(Token::Right))
        log::fatal("expected '>'", log::Span(m_file, since(start)));

    if (!expect(Token::OpenParen))
        log::fatal("expected '('", log::Span(m_file, since(start)));

    Expr* expr = parse_initial_expression();
    if (!expr)
        log::fatal("expected expression", log::Span(m_file, since(start)));

    if (!expect(Token::CloseParen))
        log::fatal("expected ')'", log::Span(m_file, since(start)));

    return CastExpr::create(
        *m_rib, SourceSpan(start, expr->span().end), type, expr);
}

Expr* Parser::parse_parentheses() {
    const SourceLocation start = loc();
    next(); // '('
    
    Expr* expr = parse_initial_expression();
    if (!expr)
        log::fatal("expected expression", log::Span(m_file, since(start)));

    const SourceLocation end = loc();
    if (!expect(Token::CloseParen))
        log::fatal("expected ')'", log::Span(m_file, since(start)));

    return ParenExpr::create(*m_rib, SourceSpan(start, end), expr);
}

Expr* Parser::parse_sizeof_operator() {
    const SourceLocation start = loc();
    next(); // 'sizeof'

    if (!expect(Token::OpenParen))
        log::fatal("expected '('", log::Span(m_file, since(start)));

    Type* type = parse_type_specifier();

    const SourceLocation end = loc();
    if (!expect(Token::CloseParen))
        log::fatal("expected ')'", log::Span(m_file, since(start)));

    return SizeofExpr::create(*m_rib, SourceSpan(start, end), type);
}

Expr* Parser::parse_named_reference() {
    uint64_t position = m_stream.position();
    next();

    if (m_allow_inits && match(Token::OpenBrace)) {
        return parse_struct_initializer(position);
    }

    m_stream.seek(position);
    SourceLocation loc_start = loc();
    std::string name = curr().value;
    next(); // id

    std::string spec = "";
    while (true) {
        if (!expect(Token::Path))
            break;

        if (!spec.empty())
            spec += "::";

        spec += name;

        if (!match(Token::Identifier))
            log::fatal("expected identifier", log::Span(m_file, since(loc())));

        name = curr().value;
        next(); // id
    }

    return RefExpr::create(*m_rib, since(loc_start), name, spec, nullptr);
}

Expr* Parser::parse_struct_initializer(uint64_t start) {
    m_stream.seek(start);
    const SourceLocation loc_start = loc();

    Type* type = parse_type_specifier();

    if (!expect(Token::OpenBrace))
        log::fatal("expected '{'", log::Span(m_file, since(loc_start)));

    std::vector<FieldInitExpr*> fields = {};

    while (!match(Token::CloseBrace)) {
        const SourceLocation field_loc_start = loc();

        if (!match(Token::Identifier))
            log::fatal("expected field identifier", log::Span(m_file, since(loc_start)));
        
        const std::string name = curr().value;
        next();

        if (!expect(Token::Colon))
            log::fatal("expected ':'", log::Span(m_file, since(loc_start)));

        Expr* expr = parse_initial_expression();
        if (!expr)
            log::fatal("expected expression", log::Span(m_file, since(loc_start)));

        fields.push_back(FieldInitExpr::create(
            *m_rib, 
            SourceSpan { loc_start, expr->span().end }, 
            expr->type(), 
            name, 
            expr
        ));

        if (match(Token::CloseBrace))
            break;

        if (!expect(Token::Comma))
            log::fatal("expected ','", log::Span(m_file, since(loc_start)));
    }

    const SourceLocation loc_end = loc();
    next(); // '}'

    return StructInitExpr::create(
        *m_rib, 
        SourceSpan { loc_start, loc_end }, 
        type, 
        fields
    );
}
