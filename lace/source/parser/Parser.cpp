//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/core/Diagnostics.h"
#include "lace/parser/Parser.h"
#include "lace/tree/Rib.h"
#include "lace/tree/Defn.h"
#include "lace/tree/Type.h"

#include <string>
#include <unordered_set>
#include <vector>

using namespace lace;

Parser::Parser(TokenStream& stream, const std::string& file)
  : m_stream(stream), m_file(file) {}

Rib* Parser::parse() {
    if (!expect("rib"))
        log::fatal("expected 'rib' identifier", log::Span { m_file, loc() });

    std::string name = parse_trail();

    m_rib = Rib::create(name, m_file);

    while (!m_stream.complete()) {
        Defn* defn = parse_initial_definition();
        if (!defn)
            log::fatal("expected definition", log::Span { m_file, loc() });

        if (defn)
            m_rib->defns().push_back(defn);
    }

    return m_rib;
}

bool Parser::is_reserved(const std::string& ident) const {
    static std::unordered_set<std::string> keywords = {
        "void", "bool", "char", 
        "s8", "s16", "s32", "s64", 
        "u8", "u16", "u32", "u64",
        "f32", "f64",
        "struct", "enum", "union",
        "let", "ret", "stop", "until", "if", "restart", "loop", "rib"
    };

    return keywords.contains(ident);
}

std::string Parser::parse_trail() {
    std::string trail = "";

    while (!match(Token::Semi)) {
        if (m_stream.complete())
            log::fatal("expected full 'rib' path", log::Span { m_file, loc() });

        if (!match(Token::Identifier))
            log::fatal("expected identifier", log::Span { m_file, loc() });
        
        trail += curr().value;
        next(); // id

        if (match(Token::Semi))
            break;

        if (!expect(Token::Path))
            log::fatal("expected '::'", log::Span { m_file, loc() });

        trail += "::";
    }

    next(); // ';'
    return trail;
}

Type* Parser::parse_type_specifier() {
    Type* type = nullptr;
    
    if (expect(Token::Star)) {
        return PointerType::get(*m_rib, parse_type_specifier());
    } else if (match(Token::Identifier)) {
        std::unordered_map<std::string, Type*> types = {
            { "void", BuiltinType::get(*m_rib, BuiltinType::Kind::Void) },
            { "bool", BuiltinType::get(*m_rib, BuiltinType::Kind::Bool) },
            { "char", BuiltinType::get(*m_rib, BuiltinType::Kind::Char) },
            { "s8", BuiltinType::get(*m_rib, BuiltinType::Kind::Int8) },
            { "s16", BuiltinType::get(*m_rib, BuiltinType::Kind::Int16) },
            { "s32", BuiltinType::get(*m_rib, BuiltinType::Kind::Int32) },
            { "s64", BuiltinType::get(*m_rib, BuiltinType::Kind::Int64) },
            { "u8", BuiltinType::get(*m_rib, BuiltinType::Kind::UInt8) },
            { "u16", BuiltinType::get(*m_rib, BuiltinType::Kind::UInt16) },
            { "u32", BuiltinType::get(*m_rib, BuiltinType::Kind::UInt32) },
            { "u64", BuiltinType::get(*m_rib, BuiltinType::Kind::UInt64) },
            { "f32", BuiltinType::get(*m_rib, BuiltinType::Kind::Float32) },
            { "f64", BuiltinType::get(*m_rib, BuiltinType::Kind::Float64) },
        };

        auto it = types.find(curr().value);
        if (it != types.end()) {
            type = it->second;
        } else {
            type = DeferredType::get(*m_rib, curr().value);
        }

        next();
    } else {
        log::fatal("expected type identifier", log::Location(m_file, loc()));
    }

    return type;
}
