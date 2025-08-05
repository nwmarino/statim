#include "lexer.h"
#include "common.h"
#include "metadata.h"
#include "token.h"

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

inline static SkBool8 is_octal(char c) {
    return c >= '0' && c <= '7';
}

inline static SkBool8 is_eof(SkLexer lexer) {
    return lexer->position >= lexer->pBuffer->size;
}

inline static i8 curr(SkLexer lexer) {
    return lexer->pBuffer->pData[lexer->position];
}

static void move(SkLexer lexer, u32 n) {
    lexer->position += n;
    lexer->meta.column += n;
}

static i8 peek(SkLexer lexer, u32 n) {
    if (lexer->position + n >= lexer->pBuffer->size)
        return '\0';

    return lexer->pBuffer->pData[lexer->position + n];
}

SkResult skInitLexer(SkInputFile file, SkLexer* pLexer) {
    assert(pLexer != NULL);

    *pLexer = malloc(sizeof(**pLexer));
    if (!*pLexer)
        return SK_FAILURE_OUT_OF_MEMORY;

    SkMetadata meta;
    meta.pFile = file;
    meta.line = 0;
    meta.column = 0;

    (*pLexer)->file = file;
    (*pLexer)->pBuffer = &file->contents;
    (*pLexer)->meta = meta;
    (*pLexer)->position = 0;
    return SK_SUCCESS;
}

SkResult skDestroyLexer(SkLexer* pLexer) {
    assert(pLexer != NULL);

    if (!*pLexer)
        return SK_FAILURE_BAD_HANDLE;

    free(*pLexer);
    *pLexer = NULL;
    return SK_SUCCESS;
}

SkToken skLexToken(SkLexer lexer) {
    while (curr(lexer) == ' ' || curr(lexer) == '\t')
        move(lexer, 1);

    SkToken token;
    token.kind = SK_TOKEN_KIND_END_OF_FILE;
    token.meta = lexer->meta;
    token.pValue = NULL;

    if (is_eof(lexer))
        return token;
    else if (curr(lexer) == '\n') {
        move(lexer, 1);
        lexer->meta.line++;
        lexer->meta.column = 1;
        return skLexToken(lexer);
    }

    switch (curr(lexer)) {
    case '+':
        if (peek(lexer, 1) == '+') {
            token.kind = SK_TOKEN_KIND_PLUS_PLUS;
            move(lexer, 2);
        } else if (peek(lexer, 1) == '=') {
            token.kind = SK_TOKEN_KIND_PLUS_EQUALS;
            move(lexer, 2);
        } else {
            token.kind = SK_TOKEN_KIND_PLUS;
            move(lexer, 1);
        }
        break;

    case '-':
        if (peek(lexer, 1) == '-') {
            token.kind = SK_TOKEN_KIND_MINUS_MINUS;
            move(lexer, 2);
        } else if (peek(lexer, 1) == '=') {
            token.kind = SK_TOKEN_KIND_MINUS_EQUALS;
            move(lexer, 2);
        } else {
            token.kind = SK_TOKEN_KIND_MINUS;
            move(lexer, 1);
        }
        break;

    case '*':
        if (peek(lexer, 1) == '=') {
            token.kind = SK_TOKEN_KIND_STAR_EQUALS;
            move(lexer, 2);
        } else {
            token.kind = SK_TOKEN_KIND_STAR;
            move(lexer, 1);
        }
        break;

    case '/':
        if (peek(lexer, 1) == '=') {
            token.kind = SK_TOKEN_KIND_SLASH_EQUALS;
            move(lexer, 2);
        } else if (peek(lexer, 1) == '/') {
            move(lexer, 2);
            while (curr(lexer) != '\n' && !is_eof(lexer))
                move(lexer, 1);

            return skLexToken(lexer);
        } else {
            token.kind = SK_TOKEN_KIND_SLASH;
            move(lexer, 1);
        }
        break;

    case '<':
        if (peek(lexer, 1) == '<') {
            token.kind = SK_TOKEN_KIND_LEFT_LEFT;
            move(lexer, 2);
        } else if (peek(lexer, 1) == '=') {
            token.kind = SK_TOKEN_KIND_LEFT_EQUALS;
            move(lexer, 2);
        } else {
            token.kind = SK_TOKEN_KIND_LEFT;
            move(lexer, 1);
        }
        break;

    case '>':
        if (peek(lexer, 1) == '>') {
            token.kind = SK_TOKEN_KIND_RIGHT_RIGHT;
            move(lexer, 2);
        } else if (peek(lexer, 1) == '=') {
            token.kind = SK_TOKEN_KIND_RIGHT_EQUALS;
            move(lexer, 2);
        } else {
            token.kind = SK_TOKEN_KIND_RIGHT;
            move(lexer, 1);
        }
        break;

    case '&':
        if (peek(lexer, 1) == '&') {
            token.kind = SK_TOKEN_KIND_AND_AND;
            move(lexer, 2);
        } else if (peek(lexer, 1) == '=') {
            token.kind = SK_TOKEN_KIND_AND_EQUALS;
            move(lexer, 2);
        } else {
            token.kind = SK_TOKEN_KIND_AND;
            move(lexer, 1);
        }
        break;

    case '|':
        if (peek(lexer, 1) == '|') {
            token.kind = SK_TOKEN_KIND_OR_OR;
            move(lexer, 2);
        } else if (peek(lexer, 1) == '=') {
            token.kind = SK_TOKEN_KIND_OR_EQUALS;
            move(lexer, 2);
        } else {
            token.kind = SK_TOKEN_KIND_OR;
            move(lexer, 1);
        }
        break;

    case '^':
        if (peek(lexer, 1) == '=') {
            token.kind = SK_TOKEN_KIND_XOR_EQUALS;
            move(lexer, 2);
        } else {
            token.kind = SK_TOKEN_KIND_XOR;
            move(lexer, 1);
        }
        break;

    case '%':
        if (peek(lexer, 1) == '=') {
            token.kind = SK_TOKEN_KIND_PERCENT_EQUALS;
            move(lexer, 2);
        } else {
            token.kind = SK_TOKEN_KIND_PERCENT;
            move(lexer, 1);
        }
        break;

    case '=':
        if (peek(lexer, 1) == '=') {
            token.kind = SK_TOKEN_KIND_EQUALS_EQUALS;
            move(lexer, 2);
        } else {
            token.kind = SK_TOKEN_KIND_EQUALS;
            move(lexer, 1);
        }
        break;

    case '!':
        if (peek(lexer, 1) == '=') {
            token.kind = SK_TOKEN_KIND_BANG_EQUALS;
            move(lexer, 2);
        } else {
            token.kind = SK_TOKEN_KIND_BANG;
            move(lexer, 1);
        }
        break;
    
    case ':':
        if (peek(lexer, 1) == ':') {
            token.kind = SK_TOKEN_KIND_PATH;
            move(lexer, 2);
        } else {
            token.kind = SK_TOKEN_KIND_COLON;
            move(lexer, 1);
        }
        break;

    case '~':
        token.kind = SK_TOKEN_KIND_TILDE;
        move(lexer, 1);
        break;

    case '(':
        token.kind = SK_TOKEN_KIND_SET_PAREN;
        move(lexer, 1);
        break;

    case ')':
        token.kind = SK_TOKEN_KIND_END_PAREN;
        move(lexer, 1);
        break;

    case '{':
        token.kind = SK_TOKEN_KIND_SET_BRACE;
        move(lexer, 1);
        break;

    case '}':
        token.kind = SK_TOKEN_KIND_END_BRACE;
        move(lexer, 1);
        break;

    case '[':
        token.kind = SK_TOKEN_KIND_SET_BRACKET;
        move(lexer, 1);
        break;

    case ']':
        token.kind = SK_TOKEN_KIND_END_BRACKET;
        move(lexer, 1);
        break;

    case '.':
        token.kind = SK_TOKEN_KIND_DOT;
        move(lexer, 1);
        break;

    case ',':
        token.kind = SK_TOKEN_KIND_COMMA;
        move(lexer, 1);
        break;

    case '@':
        token.kind = SK_TOKEN_KIND_AT;
        move(lexer, 1);
        break;
        
    case '#':
        token.kind = SK_TOKEN_KIND_HASH;
        move(lexer, 1);
        break;

    case '$':
        token.kind = SK_TOKEN_KIND_SIGN;
        move(lexer, 1);
        break;

    case ';':
        token.kind = SK_TOKEN_KIND_SEMICOLON;
        move(lexer, 1);
        break;

    case '\'': {
        token.kind = SK_TOKEN_KIND_LITERAL_CHARACTER;
        move(lexer, 1);

        char c = curr(lexer);
        if (curr(lexer) == '\\') {
            move(lexer, 1);
            
            switch (curr(lexer)) {
            case '0': c = '\0'; break;
            case 'n': c = '\n'; break;
            case 't': c = '\t'; break;
            case 'r': c = '\r'; break;
            case 'b': c = '\b'; break;
            case 'f': c = '\f'; break;
            case 'v': c = '\v'; break;
            case '\\': c = '\\'; break;
            case '\'': c = '\''; break;
            case '\"': c = '\"'; break;
            default:
                //skLogFatal("unknown escape sequence ")
                break;
            }

            token.pValue = strdup(&c);
        } else {
            token.pValue = strdup(&c);
        }

        if (peek(lexer, 1) != '\'') {
            token.kind = SK_TOKEN_KIND_APOSTROPHE;
            token.pValue = NULL;
        } else {
            move(lexer, 2);
        }
        break;
    }

    case '"': {
        token.kind = SK_TOKEN_KIND_LITERAL_STRING;
        move(lexer, 1);

        char buf[1024];
        u32 size = 0;

        char c = curr(lexer);
        while (c != '"') {
            if (c == '\\') {
                move(lexer, 1);
                switch (curr(lexer)) {
                case '0': buf[size] = '\0'; break;
                case 'n': buf[size] = '\n'; break;
                case 't': buf[size] = '\t'; break;
                case 'r': buf[size] = '\r'; break;
                case 'b': buf[size] = '\b'; break;
                case 'f': buf[size] = '\f'; break;
                case 'v': buf[size] = '\v'; break;
                case '\\': buf[size] = '\\'; break;
                case '\'': buf[size] = '\''; break;
                case '\"': buf[size] = '\"'; break;
                default:
                    //skLogFatal();
                    break;
                }
            } else {
                buf[size] = c;
            }

            move(lexer, 1);
            c = curr(lexer);
            size++;
        }

        buf[size] = '\0';
        move(lexer, 1);

        token.pValue = calloc(sizeof(char), size);
        strncpy(token.pValue, buf, size);
        break;
    }

    default: {
        if (isdigit(curr(lexer)) || curr(lexer) == '-') {
            token.kind = SK_TOKEN_KIND_LITERAL_INTEGER;

            char buf[1024];
            u32 size = 0;

            if (curr(lexer) == '-') {
                buf[size] = '-';
                size = 1;
                move(lexer, 1);
            }

            while (isdigit(curr(lexer)) || curr(lexer) == '.') {
                if (curr(lexer) == '.') {
                    if (!isdigit(peek(lexer, 1)) || token.kind == SK_TOKEN_KIND_LITERAL_FLOAT)
                        break;

                    token.kind = SK_TOKEN_KIND_LITERAL_FLOAT;
                }

                buf[size++] = curr(lexer);
                move(lexer, 1);
            }

            token.pValue = calloc(sizeof(char), size);
            strncpy(token.pValue, buf, size);
        } else if (isalpha(curr(lexer)) || curr(lexer) == '_') {
            token.kind = SK_TOKEN_KIND_IDENTIFIER;

            char buf[1024];
            u32 size = 0;

            while (isalnum(curr(lexer)) || curr(lexer) == '_') {
                buf[size++] = curr(lexer);
                move(lexer, 1);
            }

            token.pValue = calloc(sizeof(char), size);
            strncpy(token.pValue, buf, size);
        } else {
            // skLogFatal();
        }
    }

    }

    return token;
}
