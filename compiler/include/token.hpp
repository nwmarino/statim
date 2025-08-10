#ifndef STATIM_TOKEN_HPP_
#define STATIM_TOKEN_HPP_

#include "source_loc.hpp"
#include "types.hpp"

#include <string>

namespace stm {

/// Different kinds of tokens & literals.
enum TokenKind : u8 {
    TOKEN_KIND_END_OF_FILE = 0x0,
    TOKEN_KIND_IDENTIFIER,
    TOKEN_KIND_SET_PAREN,
    TOKEN_KIND_END_PAREN,
    TOKEN_KIND_SET_BRACE,
    TOKEN_KIND_END_BRACE,
    TOKEN_KIND_SET_BRACKET,
    TOKEN_KIND_END_BRACKET,
    TOKEN_KIND_EQUALS,
    TOKEN_KIND_EQUALS_EQUALS,
    TOKEN_KIND_BANG,
    TOKEN_KIND_BANG_EQUALS,
    TOKEN_KIND_PLUS,
    TOKEN_KIND_PLUS_PLUS,
    TOKEN_KIND_PLUS_EQUALS,
    TOKEN_KIND_MINUS,
    TOKEN_KIND_MINUS_MINUS,
    TOKEN_KIND_MINUS_EQUALS,
    TOKEN_KIND_STAR,
    TOKEN_KIND_STAR_EQUALS,
    TOKEN_KIND_SLASH,
    TOKEN_KIND_SLASH_EQUALS,
    TOKEN_KIND_PERCENT,
    TOKEN_KIND_PERCENT_EQUALS,
    TOKEN_KIND_AND,
    TOKEN_KIND_AND_AND,
    TOKEN_KIND_AND_EQUALS,
    TOKEN_KIND_OR,
    TOKEN_KIND_OR_OR,
    TOKEN_KIND_OR_EQUALS,
    TOKEN_KIND_XOR,
    TOKEN_KIND_XOR_EQUALS,
    TOKEN_KIND_LEFT,
    TOKEN_KIND_LEFT_LEFT,
    TOKEN_KIND_LEFT_LEFT_EQUALS,
    TOKEN_KIND_LEFT_EQUALS,
    TOKEN_KIND_RIGHT,
    TOKEN_KIND_RIGHT_RIGHT,
    TOKEN_KIND_RIGHT_RIGHT_EQUALS,
    TOKEN_KIND_RIGHT_EQUALS,
    TOKEN_KIND_ARROW,
    TOKEN_KIND_FAT_ARROW,
    TOKEN_KIND_SIGN,
    TOKEN_KIND_DOT,
    TOKEN_KIND_COMMA,
    TOKEN_KIND_COLON,
    TOKEN_KIND_PATH,
    TOKEN_KIND_SEMICOLON,
    TOKEN_KIND_APOSTROPHE,
    TOKEN_KIND_GRAVE,
    TOKEN_KIND_TILDE,
    TOKEN_KIND_INTEGER,
    TOKEN_KIND_FLOAT,
    TOKEN_KIND_CHARACTER,
    TOKEN_KIND_STRING,
};

/// Represents a token lexed from source.
struct Token final {
    SourceLocation  loc;
    TokenKind       kind;
    std::string     value;

    Token(SourceLocation loc, 
          TokenKind kind = TOKEN_KIND_END_OF_FILE, 
          const std::string& value = "") 
        : loc(loc), kind(kind), value(value) {};

    bool operator == (const Token& other) const {
        return kind == other.kind && loc == other.loc && value == other.value;
    }
};

} // namespace stm

#endif // STATIM_TOKEN_HPP_
