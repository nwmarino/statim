#include "../include/lexer.hpp"
#include "../include/logger.hpp"

#include <cassert>

using namespace stm;

static bool is_octal_digit(char c) {
    return c >= '0' && c <= '7';
}

static bool is_whitespace(char c) {
    return c == ' ' || c == '\t';
}

Lexer::Lexer(InputFile& file, const std::string& src) 
        : mFile(file), mBuf(src.empty() ? file.source() : src), mLoc(file, 1, 1) {
    mLexed.push_back(Token { mLoc });
};

const Token& Lexer::last() const {
    return mLexed.back();
}

const Token& Lexer::last(u32 n) const {
    return mLexed.at(std::max(mLexed.size() - 1 - n, u64(0)));
}

const Token& Lexer::lex() {
    if (is_eof())
        return mLexed.back();
    
    if (curr() == '\n') {
        mPos++;
        end_line();
        return lex();
    } else if (is_whitespace(curr())) {
        while (is_whitespace(curr()))
            move();

        return lex();
    }

    mLexed.push_back(Token { mLoc });
    Token& token = mLexed.back();

    switch (curr()) {
    case '+':
        if (peek() == '+') {
            token.kind = TOKEN_KIND_PLUS_PLUS;
            move(2);
        } else if (peek() == '=') {
            token.kind = TOKEN_KIND_PLUS_EQUALS;
            move(2);
        } else {
            token.kind = TOKEN_KIND_PLUS;;
            move();
        }
        break;

    case '-':
        if (peek() == '-') {
            token.kind = TOKEN_KIND_MINUS_MINUS;
            move(2);
        } else if (peek() == '=') {
            token.kind = TOKEN_KIND_MINUS_EQUALS;
            move(2);
        } else if (peek() == '>') {
            token.kind = TOKEN_KIND_ARROW;
            move(2);
        } else {
            token.kind = TOKEN_KIND_MINUS;
            move(1);
        }
        break;

    case '*':
        if (peek() == '=') {
            token.kind = TOKEN_KIND_STAR_EQUALS;
            move(2);
        } else {
            token.kind = TOKEN_KIND_STAR;
            move();
        }
        break;

    case '/':
        if (peek() == '/') {
            move(2);
            while (curr() != '\n' && curr() != '\0')
                move();
            
            return lex();
        } else if (peek() == '=') {
            token.kind = TOKEN_KIND_SLASH_EQUALS;
            move(2);
        } else {
            token.kind = TOKEN_KIND_SLASH;
            move();
        }
        break;

    case '<':
        if (peek() == '<') {
            if (peek(2) == '=') {
                token.kind = TOKEN_KIND_LEFT_LEFT_EQUALS;
                move(3);
            } else {
                token.kind = TOKEN_KIND_LEFT_LEFT;
                move(2);
            }
        } else if (peek() == '=') {
            token.kind = TOKEN_KIND_LEFT_EQUALS;
            move(2);
        } else {
            token.kind = TOKEN_KIND_LEFT;
            move();
        }
        break;

    case '>':
        if (peek() == '>') {
            if (peek(2) == '=') {
                token.kind = TOKEN_KIND_RIGHT_RIGHT_EQUALS;
                move(3);
            } else {
                token.kind = TOKEN_KIND_RIGHT_RIGHT;
                move(2);
            }
        } else if (peek() == '=') {
            token.kind = TOKEN_KIND_RIGHT_EQUALS;
            move(2);
        } else {
            token.kind = TOKEN_KIND_RIGHT;
            move();
        }
        break;

    case '&':
        if (peek() == '&') {
            token.kind = TOKEN_KIND_AND_AND;
            move(2);
        } else if (peek() == '=') {
            token.kind = TOKEN_KIND_AND_EQUALS;
            move(2);
        } else {
            token.kind = TOKEN_KIND_AND;
            move();
        }
        break;

    case '|':
        if (peek() == '|') {
            token.kind = TOKEN_KIND_OR_OR;
            move(2);
        } else if (peek() == '=') {
            token.kind = TOKEN_KIND_OR_EQUALS;
            move(2);
        } else {
            token.kind = TOKEN_KIND_OR;
            move();
        }
        break;

    case '^':
        if (peek() == '=') {
            token.kind = TOKEN_KIND_XOR_EQUALS;
            move(2);
        } else {
            token.kind = TOKEN_KIND_XOR;
            move();
        }
        break;

    case '%':
        if (peek() == '=') {
            token.kind = TOKEN_KIND_PERCENT_EQUALS;
            move(2);
        } else {
            token.kind = TOKEN_KIND_PERCENT;
            move();
        }
        break;

    case '=':
        if (peek() == '=') {
            token.kind = TOKEN_KIND_EQUALS_EQUALS;
            move(2);
        } else if (peek() == '>') {
            token.kind = TOKEN_KIND_FAT_ARROW;
            move(2);
        } else {
            token.kind = TOKEN_KIND_EQUALS;
            move();
        }
        break;

    case '!':
        if (peek() == '=') {
            token.kind = TOKEN_KIND_BANG_EQUALS;
            move(2);
        } else {
            token.kind = TOKEN_KIND_BANG;
            move();
        }
        break;

    case ':':
        if (peek() == ':') {
            token.kind = TOKEN_KIND_PATH;
            move(2);
        } else {
            token.kind = TOKEN_KIND_COLON;
            move();
        }
        break;

    case '~':
        token.kind = TOKEN_KIND_TILDE;
        move();
        break;

    case '(':
        token.kind = TOKEN_KIND_SET_PAREN;
        move();
        break;

    case ')':
        token.kind = TOKEN_KIND_END_PAREN;
        move();
        break;

    case '{':
        token.kind = TOKEN_KIND_SET_BRACE;
        move();
        break;

    case '}':
        token.kind = TOKEN_KIND_END_BRACE;
        move();
        break;

    case '[':
        token.kind = TOKEN_KIND_SET_BRACKET;
        move();
        break;

    case ']':
        token.kind = TOKEN_KIND_END_BRACKET;
        move();
        break;

    case '.':
        token.kind = TOKEN_KIND_DOT;
        move();
        break;

    case ',':
        token.kind = TOKEN_KIND_COMMA;
        move();
        break;

    case '$':
        token.kind = TOKEN_KIND_SIGN;
        move();
        break;

    case ';':
        token.kind = TOKEN_KIND_SEMICOLON;
        move();
        break;

    case '`':
        token.kind = TOKEN_KIND_GRAVE;
        move();
        break;

    case '\'':
        move();
        token.kind = TOKEN_KIND_CHARACTER;

        if (curr() == '\\') {
            move();
            switch (curr()) {
                case '0': token.value = "\0"; break;
                case 'n': token.value = "\n"; break;
                case 't': token.value = "\t"; break;
                case 'r': token.value = "\r"; break;
                case 'b': token.value = "\b"; break;
                case 'f': token.value = "\f"; break;
                case 'v': token.value = "\v"; break;
                case '\\': token.value = "\\"; break;
                case '\'': token.value = "\'"; break;
                case '\"': token.value = "\""; break;
                default: 
                    logger_fatal("unknown escape sequence: " + std::to_string(curr()), &mLoc);
            }
        } else
            token.value = curr();

        if (peek() != '\'')
            token.kind = TOKEN_KIND_APOSTROPHE;
        else
            move(2);
            
        break;

    case '"':
        move();
        token.kind = TOKEN_KIND_STRING;
        token.value = "";

        while (curr() != '"') {
            if (curr() == '\\') {
                move();
                switch (curr()) {
                    case '0': token.value += '\0'; break;
                    case 'n': token.value += '\n'; break;
                    case 't': token.value += '\t'; break;
                    case 'r': token.value += '\r'; break;
                    case 'b': token.value += '\b'; break;
                    case 'f': token.value += '\f'; break;
                    case 'v': token.value += '\v'; break;
                    case '\\': token.value += '\\'; break;
                    case '\'': token.value += '\''; break;
                    case '\"': token.value += '\"'; break;
                    default: {
                        if (is_octal_digit(curr())) {
                            i32 oct_val = 0;
                            i32 digits = 0;

                            while (digits < 3 && is_octal_digit(curr())) {
                                oct_val = (oct_val << 3) + (curr() - '0');
                                move();
                                digits++;
                            }

                            token.value += static_cast<char>(oct_val);
                            continue;
                        } else {
                            logger_fatal("unknown escape sequence: " + std::to_string(curr()), &mLoc);
                        }
                    }
                }
            } else {
                token.value += curr();  
            }
            move();
        }
        move();
        break;
    
    default: {
        if (std::isdigit(curr()) || curr() == '-') {
            token.kind = TOKEN_KIND_INTEGER;

            if (curr() == '-') {
                token.value += curr();
                move();
            }

            while (std::isdigit(curr()) || curr() == '.') {
                if (curr() == '.') {
                    if (!std::isdigit(peek()) || token.kind == TOKEN_KIND_FLOAT)
                        break;

                    token.kind = TOKEN_KIND_FLOAT;
                }
                    
                token.value += curr();
                move();
            }
        } else if (std::isalpha(curr()) || curr() == '_') {
            token.kind = TOKEN_KIND_IDENTIFIER;
            
            while (std::isalnum(curr()) || curr() == '_') {
                token.value += curr();
                move();
            }
        } else {
            logger_fatal("unrecognized token: " + std::to_string(curr()), &mLoc);
        }
    }

    }

    return token;
}

bool Lexer::is_eof() const { 
    return mPos >= mBuf.size(); 
}

char Lexer::curr() const { 
    return mBuf.at(mPos); 
}

char Lexer::peek(u32 n) const {
    if (mPos + n >= mBuf.size())
        return '\0';

    return mBuf[mPos + n];
}

void Lexer::end_line() {
    mLoc.line++;
    mLoc.column = 1;
}

void Lexer::move(u32 n) {
    mPos += n;
    mLoc.column++;
}
