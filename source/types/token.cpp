#include "token.hpp"

using namespace stm;

std::string stm::token_kind_to_string(TokenKind kind) {
    switch (kind) {
        case TOKEN_KIND_END_OF_FILE: return "eof";
        case TOKEN_KIND_IDENTIFIER: return "identifier";
        case TOKEN_KIND_SET_PAREN: return "(";
        case TOKEN_KIND_END_PAREN: return ")";
        case TOKEN_KIND_SET_BRACE: return "{";
        case TOKEN_KIND_END_BRACE: return "}";
        case TOKEN_KIND_SET_BRACKET: return "[";
        case TOKEN_KIND_END_BRACKET: return "]";
        case TOKEN_KIND_EQUALS: return "=";
        case TOKEN_KIND_EQUALS_EQUALS: return "==";
        case TOKEN_KIND_BANG: return "!";
        case TOKEN_KIND_BANG_EQUALS: return "!=";
        case TOKEN_KIND_PLUS: return "+";
        case TOKEN_KIND_PLUS_PLUS: return "++";
        case TOKEN_KIND_PLUS_EQUALS: return "+=";
        case TOKEN_KIND_MINUS: return "-";
        case TOKEN_KIND_MINUS_MINUS: return "--";
        case TOKEN_KIND_MINUS_EQUALS: return "-=";
        case TOKEN_KIND_STAR: return "*";
        case TOKEN_KIND_STAR_EQUALS: return "*=";
        case TOKEN_KIND_SLASH: return "/";
        case TOKEN_KIND_SLASH_EQUALS: return "/=";
        case TOKEN_KIND_PERCENT: return "%";
        case TOKEN_KIND_PERCENT_EQUALS: return "%=";
        case TOKEN_KIND_AND: return "&";
        case TOKEN_KIND_AND_AND: return "&&";
        case TOKEN_KIND_AND_EQUALS: return "&=";
        case TOKEN_KIND_OR: return "|";
        case TOKEN_KIND_OR_OR: return "||";
        case TOKEN_KIND_OR_EQUALS: return "|=";
        case TOKEN_KIND_XOR: return "^";
        case TOKEN_KIND_XOR_EQUALS: return "^=";
        case TOKEN_KIND_LEFT: return "<";
        case TOKEN_KIND_LEFT_LEFT: return "<<";
        case TOKEN_KIND_LEFT_LEFT_EQUALS: return "<<=";
        case TOKEN_KIND_LEFT_EQUALS: return "<=";
        case TOKEN_KIND_RIGHT: return ">";
        case TOKEN_KIND_RIGHT_RIGHT: return ">>";
        case TOKEN_KIND_RIGHT_RIGHT_EQUALS: return ">>=";
        case TOKEN_KIND_RIGHT_EQUALS: return ">=";
        case TOKEN_KIND_ARROW: return "->";
        case TOKEN_KIND_FAT_ARROW: return "=>";
        case TOKEN_KIND_SIGN: return "$";
        case TOKEN_KIND_DOT: return ".";
        case TOKEN_KIND_COMMA: return ",";
        case TOKEN_KIND_COLON: return ":";
        case TOKEN_KIND_PATH: return "::";
        case TOKEN_KIND_SEMICOLON: return ";";
        case TOKEN_KIND_APOSTROPHE: return "'";
        case TOKEN_KIND_GRAVE: return "`";
        case TOKEN_KIND_TILDE: return "~";
        case TOKEN_KIND_INTEGER: return "integer";
        case TOKEN_KIND_FLOAT: return "float";
        case TOKEN_KIND_CHARACTER: return "character";
        case TOKEN_KIND_STRING: return "string";
        default: return "unknown";
    }
}
