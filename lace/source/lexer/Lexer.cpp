//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/core/Diagnostics.h"
#include "lace/lexer/Lexer.h"
#include "lace/lexer/Token.h"

using namespace lace;

/// Test if |c| is an octal digit.
static inline bool isOctalDigit(char c) {
    return '0' <= c && c <= '7';
}

/// Test if |c| is a space or tab.
static inline bool isWhitespace(char c) {
    return c == ' ' || c == '\t';
}

/// Test if |kind| is for a compound token i.e. a symbolic token containing more than one symbol.
static inline bool isCompoundToken(Token::Kind kind) {
    switch (kind) {
        case Token::EqEq:
        case Token::BangEq:
        case Token::LeftEq:
        case Token::LeftLeft:
        case Token::RightEq:
        case Token::RightRight:
        case Token::AndAnd:
        case Token::OrOr:
        case Token::Arrow:
        case Token::Path:
            return true;
        default:
            return false;
    }
}

void Lexer::lex(Token& token) {
    token.value.clear();

    if (isEof()) {
        token.kind = Token::EndOfFile;
        token.loc = m_loc;
        return;
    }

    if (curr() == '\n') {
        ++m_cursor;
        endLine();
        return lex(token);
    } else if (isWhitespace(curr())) {
        while (isWhitespace(curr()))
            move();

        return lex(token);
    }

    token.loc = m_loc;

    switch (curr()) {
        case '+':
            token.kind = Token::Plus;
            move();
            break;

        case '-':
            if (peek() == '>') {
                token.kind = Token::Arrow;
                move(2);
            } else {
                token.kind = Token::Minus;
                move();
            }

            break;

        case '*':
            token.kind = Token::Star;
            move();
            break;

        case '/':
            if (peek() == '/') {
                move(2);
                while (curr() != '\n' && curr() != '\0')
                    move();

                if (curr() == '\n') {
                    ++m_cursor;
                    endLine();
                }

                return lex(token);
            } else {
                move();
                token.kind = Token::Slash;
            }

            break;

        case '%':
            token.kind = Token::Percent;
            move();
            break;
        
        case '<':
            if (peek() == '<') {
                token.kind = Token::LeftLeft;
                move(2);
            } else if (peek() == '=') {
                token.kind = Token::LeftEq;
                move(2);
            } else {
                token.kind = Token::Left;
                move();
            }

            break;

        case '>':
            if (peek() == '>') {
                token.kind = Token::RightRight;
                move(2);
            } else if (peek() == '=') {
                token.kind = Token::RightEq;
                move(2);
            } else {
                token.kind = Token::Right;
                move();
            }

            break;

        case '&':
            if (peek() == '&') {
                token.kind = Token::AndAnd;
                move(2);
            } else {
                token.kind = Token::And;
                move();
            }

            break;

        case '|':
            if (peek() == '|') {
                token.kind = Token::OrOr;
                move(2);
            } else {
                token.kind = Token::Or;
                move();
            }

            break;

        case '^':
            token.kind = Token::Xor;
            move();
            break;

        case '=':
            if (peek() == '=') {
                token.kind = Token::EqEq;
                move(2);
            } else {
                token.kind = Token::Eq;
                move();
            }

            break;

        case '!':
            if (peek() == '=') {
                token.kind = Token::BangEq;
                move(2);
            } else {
                token.kind = Token::Bang;
                move();
            }

            break;

        case ':':
            if (peek() == ':') {
                token.kind = Token::Path;
                move(2);
            } else {
                token.kind = Token::Colon;
                move();
            }

            break;
        
        case '~':
            token.kind = Token::Tilde;
            move();
            break;

        case '(':
            token.kind = Token::OpenParen;
            move();
            break;

        case ')':
            token.kind = Token::CloseParen;
            move();
            break;

        case '{':
            token.kind = Token::OpenBrace;
            move();
            break;

        case '}':
            token.kind = Token::CloseBrace;
            move();
            break;

        case '[':
            token.kind = Token::OpenBrack;
            move();
            break;

        case ']':
            token.kind = Token::CloseBrack;
            move();
            break;

        case ',':
            token.kind = Token::Comma;
            move();
            break;

        case ';':
            token.kind = Token::Semi;
            move();
            break;
        
        case '$':
            token.kind = Token::Sign;
            move();
            break;

        case '.':
            if (std::isdigit(peek())) {
                token.kind = Token::Float;
                token.value = ".";
                move();

                while (std::isdigit(curr())) {
                    token.value += curr();
                    move();
                }
            } else {
                token.kind = Token::Dot;
                move();
            }

            break;

        case '\'':
            move(); // '
            token.kind = Token::Character;

            if (curr() == '\\') {
                move(); // '\'

                switch (curr()) {
                    case '0':
                        token.value = "\0"; 
                        break;
                    case 'n':
                        token.value = "\n"; 
                        break;
                    case 't':
                        token.value = "\t"; 
                        break;
                    case 'r':
                        token.value = "\r"; 
                        break;
                    case 'b':
                        token.value = "\b"; 
                        break;
                    case 'f':
                        token.value = "\f"; 
                        break;
                    case 'v':
                        token.value = "\v"; 
                        break;
                    case '\\':
                        token.value = "\\"; 
                        break;
                    case '\'':
                        token.value = "\'"; 
                        break;
                    case '\"':
                        token.value = "\""; 
                        break;
                    default: {
                        log::fatal("unknown character escape sequence: " + 
                            std::to_string(curr()), log::Location(m_filename, m_loc));
                    }
                }
            } else {
                token.value = curr();
            }

            move(2);    
            break;

        case '"':
            move(); // "
            token.kind = Token::String;
            token.value = "";

            while (curr() != '"') {
                if (curr() == '\\') {
                    move(); // '\'

                    switch (curr()) {
                        case '0': 
                            token.value += '\0'; 
                            break;
                        case 'n': 
                            token.value += '\n'; 
                            break;
                        case 't': 
                            token.value += '\t'; 
                            break;
                        case 'r': 
                            token.value += '\r'; 
                            break;
                        case 'b': 
                            token.value += '\b'; 
                            break;
                        case 'f': 
                            token.value += '\f'; 
                            break;
                        case 'v': 
                            token.value += '\v'; 
                            break;
                        case '\\': 
                            token.value += '\\'; 
                            break;
                        case '\'': 
                            token.value += '\''; 
                            break;
                        case '\"': 
                            token.value += '\"'; 
                            break;
                        default: if (isOctalDigit(curr())) {
                            int32_t oct_val = 0, digits = 0;
                            while (digits < 3 && isOctalDigit(curr())) {
                                oct_val = (oct_val << 3) + (curr() - '0');
                                move();
                                digits++;
                            }

                            token.value += static_cast<char>(oct_val);
                            continue;
                        } else {
                            log::fatal("unknown escape sequence: " + 
                                std::to_string(curr()), log::Location(m_filename, m_loc));
                        }
                    }
                } else {
                    token.value += curr();  
                }

                move();
            }

            move();
            break;

        default: if (std::isdigit(curr()) || curr() == '-') {
            token.kind = Token::Integer;

            if (curr() == '-') {
                token.value += curr();
                move();
            }

            while (std::isdigit(curr()) || curr() == '.') {
                if (curr() == '.') {
                    if (token.kind == Token::Float)
                        break;

                    token.kind = Token::Float;
                }
                    
                token.value += curr();
                move();
            }
        } else if (std::isalpha(curr()) || curr() == '_') {
            token.kind = Token::Identifier;
            
            while (std::isalnum(curr()) || curr() == '_') {
                token.value += curr();
                move();
            }
        } else {
            log::fatal("unrecognized token: " + std::to_string(curr()), 
                log::Location(m_filename, m_loc));
        }
    }
}
