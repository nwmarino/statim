//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/core/Diagnostics.h"
#include "lace/parser/Parser.h"
#include "lace/tree/AST.h"
#include "lace/tree/Defn.h"
#include "lace/tree/Type.h"

#include <string>
#include <unordered_set>

using namespace lace;

Parser::Parser(TokenStream& stream, const std::string& file)
  : m_stream(stream), m_file(file) {}

AST* Parser::parse() {
    m_ast = AST::create(m_file);
    m_context = &m_ast->get_context();
    m_scope = m_ast->get_scope();

    while (!m_stream.complete()) {
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
        if (type.isMut()) {
            log::warn("duplicate 'mut' keyword", log::Location(m_file, loc()));
        } else {
            type.withMut();
        }
    }
    
    if (expect(Token::Star)) {
        type.setType(PointerType::get(*m_context, parse_type_specifier()));
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

        type.setType(ArrayType::get(*m_context, parse_type_specifier(), size));
        return type;
    } else if (match(Token::Identifier)) {
        std::unordered_map<std::string, const Type*> types = {
            { "void", BuiltinType::get(*m_context, BuiltinType::Kind::Void) },
            { "bool", BuiltinType::get(*m_context, BuiltinType::Kind::Bool) },
            { "char", BuiltinType::get(*m_context, BuiltinType::Kind::Char) },
            { "s8", BuiltinType::get(*m_context, BuiltinType::Kind::Int8) },
            { "s16", BuiltinType::get(*m_context, BuiltinType::Kind::Int16) },
            { "s32", BuiltinType::get(*m_context, BuiltinType::Kind::Int32) },
            { "s64", BuiltinType::get(*m_context, BuiltinType::Kind::Int64) },
            { "u8", BuiltinType::get(*m_context, BuiltinType::Kind::UInt8) },
            { "u16", BuiltinType::get(*m_context, BuiltinType::Kind::UInt16) },
            { "u32", BuiltinType::get(*m_context, BuiltinType::Kind::UInt32) },
            { "u64", BuiltinType::get(*m_context, BuiltinType::Kind::UInt64) },
            { "f32", BuiltinType::get(*m_context, BuiltinType::Kind::Float32) },
            { "f64", BuiltinType::get(*m_context, BuiltinType::Kind::Float64) },
        };

        auto it = types.find(curr().value);
        if (it != types.end()) {
            type.setType(it->second);
        } else {
            type.setType(DeferredType::get(*m_context, curr().value));
        }

        next();
        return type;
    } else {
        log::fatal("expected type identifier", log::Location(m_file, loc()));
    }
}
