//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lace/core/Diagnostics.h"
#include "lace/lexer/Lexer.h"
#include "lace/parser/Parser.h"
#include "lace/tree/AST.h"
#include "lace/tree/Defn.h"
#include "lace/tree/Type.h"

#include <string>
#include <unordered_set>

using namespace lace;

Parser::Parser(const std::string& source, const std::string& path)
  : m_file(path), m_lexer(source, path) {}

AST* Parser::parse() {
    m_ast = AST::create(m_file);
    m_context = &m_ast->get_context();
    m_scope = m_ast->get_scope();

    next(); // Lex the first token.

    while (!m_lexer.isEof()) {
        Defn* defn = parse_initial_definition();
        if (!defn)
            log::fatal("expected definition", log::Location(m_file, loc()));

        m_ast->get_defns().push_back(defn);
    }

    return m_ast;
}

bool Parser::is_reserved(const std::string& ident) const {
    static std::unordered_set<std::string> keywords = {
        "void", "bool", "char", 
        "s8", "s16", "s32", "s64", 
        "u8", "u16", "u32", "u64",
        "f32", "f64",
        "mut", "struct", "enum", "union",
        "let", "ret", "stop", "until", "if", "restart",
    };

    return keywords.contains(ident);
}

QualType Parser::parse_type_specifier() {
    QualType type = {};

    while (expect("mut")) {
        if (type.is_mut()) {
            log::warn("duplicate 'mut' keyword", log::Location(m_file, loc()));
        } else {
            type.with_mut();
        }
    }
    
    if (expect(Token::Star)) {
        type.set_type(PointerType::get(*m_context, parse_type_specifier()));
        return type;
    } else if (expect(Token::OpenBrack)) {
        if (!match(Token::Integer))
            log::fatal("expected integer after '['", log::Location(m_file, loc()));

        int32_t size = std::stoi(curr().value);
        if (size <= 0)
            log::fatal("array size must be greater than 0", log::Location(m_file, loc()));

        next();

        if (!expect(Token::CloseBrack))
            log::fatal("expected ']'", log::Location(m_file, loc()));

        type.set_type(ArrayType::get(*m_context, parse_type_specifier(), size));
        return type;
    } else if (match(Token::Identifier)) {
        std::unordered_map<std::string, const Type*> types = {
            { "void", BuiltinType::get(*m_context, BuiltinType::Void) },
            { "bool", BuiltinType::get(*m_context, BuiltinType::Bool) },
            { "char", BuiltinType::get(*m_context, BuiltinType::Char) },
            { "s8", BuiltinType::get(*m_context, BuiltinType::Int8) },
            { "s16", BuiltinType::get(*m_context, BuiltinType::Int16) },
            { "s32", BuiltinType::get(*m_context, BuiltinType::Int32) },
            { "s64", BuiltinType::get(*m_context, BuiltinType::Int64) },
            { "u8", BuiltinType::get(*m_context, BuiltinType::UInt8) },
            { "u16", BuiltinType::get(*m_context, BuiltinType::UInt16) },
            { "u32", BuiltinType::get(*m_context, BuiltinType::UInt32) },
            { "u64", BuiltinType::get(*m_context, BuiltinType::UInt64) },
            { "f32", BuiltinType::get(*m_context, BuiltinType::Float32) },
            { "f64", BuiltinType::get(*m_context, BuiltinType::Float64) },
        };

        auto it = types.find(curr().value);
        if (it != types.end()) {
            type.set_type(it->second);
        } else {
            type.set_type(DeferredType::get(*m_context, curr().value));
        }

        next();
        return type;
    } else {
        log::fatal("expected type identifier", log::Location(m_file, loc()));
    }
}
