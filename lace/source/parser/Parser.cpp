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
    m_scope = m_ast->scope();

    while (!m_stream.complete()) {
        Defn* defn = parse_initial_definition();
        if (!defn)
            log::fatal("expected definition", log::Location(m_file, loc()));

        m_ast->defns().push_back(defn);
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

Type* Parser::parse_type_specifier() {
    Type* type = nullptr;
    
    if (expect(Token::Star)) {
        return PointerType::get(*m_ast, parse_type_specifier());
    } else if (match(Token::Identifier)) {
        std::unordered_map<std::string, Type*> types = {
            { "void", BuiltinType::get(*m_ast, BuiltinType::Kind::Void) },
            { "bool", BuiltinType::get(*m_ast, BuiltinType::Kind::Bool) },
            { "char", BuiltinType::get(*m_ast, BuiltinType::Kind::Char) },
            { "s8", BuiltinType::get(*m_ast, BuiltinType::Kind::Int8) },
            { "s16", BuiltinType::get(*m_ast, BuiltinType::Kind::Int16) },
            { "s32", BuiltinType::get(*m_ast, BuiltinType::Kind::Int32) },
            { "s64", BuiltinType::get(*m_ast, BuiltinType::Kind::Int64) },
            { "u8", BuiltinType::get(*m_ast, BuiltinType::Kind::UInt8) },
            { "u16", BuiltinType::get(*m_ast, BuiltinType::Kind::UInt16) },
            { "u32", BuiltinType::get(*m_ast, BuiltinType::Kind::UInt32) },
            { "u64", BuiltinType::get(*m_ast, BuiltinType::Kind::UInt64) },
            { "f32", BuiltinType::get(*m_ast, BuiltinType::Kind::Float32) },
            { "f64", BuiltinType::get(*m_ast, BuiltinType::Kind::Float64) },
        };

        auto it = types.find(curr().value);
        if (it != types.end()) {
            type = it->second;
        } else {
            type = DeferredType::get(*m_ast, curr().value);
        }

        next();
    } else {
        log::fatal("expected type identifier", log::Location(m_file, loc()));
    }

    return type;
}
