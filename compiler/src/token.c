#include "../include/token.h"

STM_API_ATTR const char* STM_API_CALL stmStringifyToken(StmTokenKind kind) {
    switch (kind) {
    case STM_TOKEN_KIND_IDENTIFIER: return "identifier";
    case STM_TOKEN_KIND_LINE_COMMENT: return "line comment";
    case STM_TOKEN_KIND_BLOCK_COMMENT: return "block comment";
    case STM_TOKEN_KIND_END_OF_FILE: return "end of file";
    case STM_TOKEN_KIND_SET_BRACE: return "{";
    case STM_TOKEN_KIND_END_BRACE: return "}";
    case STM_TOKEN_KIND_SET_PAREN: return "(";
    case STM_TOKEN_KIND_END_PAREN: return ")";
    case STM_TOKEN_KIND_SET_BRACKET: return "[";
    case STM_TOKEN_KIND_END_BRACKET: return "]";
    case STM_TOKEN_KIND_AT: return "@";
    case STM_TOKEN_KIND_HASH: return "#";
    case STM_TOKEN_KIND_SIGN: return "$";
    case STM_TOKEN_KIND_PLUS: return "+";
    case STM_TOKEN_KIND_PLUS_PLUS: return "++";
    case STM_TOKEN_KIND_PLUS_EQUALS: return "+=";
    case STM_TOKEN_KIND_MINUS: return "-";
    case STM_TOKEN_KIND_MINUS_MINUS: return "--";
    case STM_TOKEN_KIND_MINUS_EQUALS: return "-=";
    case STM_TOKEN_KIND_STAR: return "*";
    case STM_TOKEN_KIND_STAR_EQUALS: return "*=";
    case STM_TOKEN_KIND_SLASH: return "/";
    case STM_TOKEN_KIND_SLASH_EQUALS: return "/=";
    case STM_TOKEN_KIND_PERCENT: return "%";
    case STM_TOKEN_KIND_PERCENT_EQUALS: return "%=";
    case STM_TOKEN_KIND_EQUALS: return "=";
    case STM_TOKEN_KIND_EQUALS_EQUALS: return "==";
    case STM_TOKEN_KIND_BANG: return "!";
    case STM_TOKEN_KIND_BANG_EQUALS: return "!=";
    case STM_TOKEN_KIND_LEFT: return "<";
    case STM_TOKEN_KIND_LEFT_LEFT: return "<<";
    case STM_TOKEN_KIND_LEFT_EQUALS: return "<=";
    case STM_TOKEN_KIND_RIGHT: return ">";
    case STM_TOKEN_KIND_RIGHT_RIGHT: return ">=";
    case STM_TOKEN_KIND_RIGHT_EQUALS: return ">=";
    case STM_TOKEN_KIND_AND: return "&";
    case STM_TOKEN_KIND_AND_AND: return "&&";
    case STM_TOKEN_KIND_AND_EQUALS: return "&=";
    case STM_TOKEN_KIND_OR: return "|";
    case STM_TOKEN_KIND_OR_OR: return "||";
    case STM_TOKEN_KIND_OR_EQUALS: return "|=";
    case STM_TOKEN_KIND_XOR: return "^";
    case STM_TOKEN_KIND_XOR_EQUALS: return "^=";
    case STM_TOKEN_KIND_SKINNY_ARROW: return "->";
    case STM_TOKEN_KIND_FAT_ARROW: return "=>";
    case STM_TOKEN_KIND_DOT: return ".";
    case STM_TOKEN_KIND_COMMA: return ",";
    case STM_TOKEN_KIND_COLON: return ":";
    case STM_TOKEN_KIND_PATH: return "::";
    case STM_TOKEN_KIND_SEMICOLON: return ";";
    case STM_TOKEN_KIND_APOSTROPHE: return "'";
    case STM_TOKEN_KIND_TILDE: return "~";
    case STM_TOKEN_KIND_GRAVE: return "`";
    case STM_TOKEN_KIND_LITERAL_INTEGER: return "integer";
    case STM_TOKEN_KIND_LITERAL_FLOAT: return "float";
    case STM_TOKEN_KIND_LITERAL_CHARACTER: return "character";
    case STM_TOKEN_KIND_LITERAL_STRING: return "string";
    }
}
