#include <cstdio>
#include <string>

#include "../include/core/Logger.h"
#include "../include/token/Token.h"
#include "../include/token/Tokenizer.h"

Tokenizer::Tokenizer(const std::string src, const std::string filename, std::size_t len)
  : src(src), filename(filename), len(len), prev('\0'), iter(0), line(1), col(1) {};

const struct Token Tokenizer::advance_token() {
  TokenKind kind = Eof;
  Metadata meta(filename, line, col);
  std::string value;
  LiteralKind lit_kind;

  if (iter >= len) {
    return Token(TokenKind::Eof, meta);
  }

  const char chr = src[iter];
  switch (chr)
  {
    /// Whitespace and new lines.
    case ' ':
    case '\t':
    case '\n':
      iter++;
      col++;
      if (chr == '\n') {
        line++;
        col = 1;
      }
      return advance_token();

    /// Slash, division assignment, or comment. Currently discard comments until doc support.
    case '/':
      if (peek() == '/') {
        while (src[iter] != '\n') {
          iter++;
          col++;
        }
        return advance_token();
      } else if (peek() == '*') {
        while (iter < len && src[iter] != '*' && peek() != '/') {
          iter++;
          col++;
        }
        return advance_token();
      } else if (peek() == '=') {
        kind = SlashEq;
        iter++;
        col++;
        break;
      }
      kind = Slash;
      break;

    /// Multiplication or multiplicative assignment.
    case '*':
      kind = Star;
      if (peek() == '=') {
        kind = StarEq;
        iter++;
        col++;
      }
      break;

    /// Addition or additive assignment.
    case '+':
      kind = Add;
      if (peek() == '=') {
        kind = AddEq;
        iter++;
        col++;
      }
      break;

    /// Negative numerical literals, subtraction, subtraction assignment or thin arrow.
    case '-':
      if (isdigit(peek())) {
        kind = Literal;
        lit_kind = Integer;
        value.push_back(src[iter]);
        iter++;
        col++;

        while (isdigit(src[iter]) || src[iter] == '.') {
          if (src[iter] == '.' && lit_kind == Integer) {
            lit_kind = Float;
          }
          value.push_back(src[iter]);
          iter++;
          col++;
        }
        return Token(kind, meta, value, lit_kind);
      }
      kind = Sub;
      if (peek() == '>') {
        kind = Arrow;
        iter++;
        col++;
      } else if (peek() == '=') {
        kind = SubEq;
        iter++;
        col++;
      }
      break;
    
    /// Char literals.
    case '\'':
      kind = Literal;
      lit_kind = Char;
      iter++;
      col++;

      value = src[iter];

      if (peek() != '\'') {
        panic("bad char literal", meta);
      }

      iter++;
      col++;
      break;

    /// String literals.
    case '"':
      kind = Literal;
      lit_kind = String;

      while (peek() != '"') {
        iter++;
        col++;
        value.push_back(src[iter]);
      }
      iter++;
      col++;
      break;

    /// Dots.
    case '.':
      kind = Dot;
      break;

    /// Equals, fat arrows, or comparison.
    case '=':
      if (peek() != '>' && peek() != '=') {
        kind = Eq;
        break;
      }

      kind = (peek() == '>') ? FatArrow : EqEq;
      iter++;
      col++;
      break;

    /// Less than (equals).
    case '<':
      if (peek() == '=') {
        kind = LessThanEq;
        iter++;
        col++;
      } else {
        kind = LessThan;
      }
      break;

    /// Greater than (equals) or right shift.
    case '>':
      if (peek() == '=') {
        kind = GreaterThanEq;
        iter++;
        col++;
      } else {
        kind = GreaterThan;
      }
      break;

    /// Logical and.
    case '&':
      if (peek() == '&') {
        kind = AndAnd;
        iter++;
        col++;
      }
      break;

    /// Logical or.
    case '|':
      if (peek() == '|') {
        kind = OrOr;
        iter++;
        col++;
      }
      break;

    /// Colon or Path.
    case ':':
      kind = Colon;
      if (peek() == ':') {
        kind = Path;
        iter++;
        col++;
      }
      break;

    /// One-character tokens.
    case '{': kind = OpenBrace; break;
    case '}': kind = CloseBrace; break;
    case '(': kind = OpenParen; break;
    case ')': kind = CloseParen; break;
    case '[': kind = OpenBracket; break;
    case ']': kind = CloseBracket; break;
    case ',': kind = Comma; break;
    case ';': kind = Semi; break;
    case '@': kind = At; break;
    case '#': kind = Hash; break;
    case '!': kind = Not; break;

    /// Identifiers, numerics, unknowns.
    default:
      if (isalpha(chr) || chr == '_') {
        kind = Identifier;
        
        while (isalnum(src[iter]) || src[iter] == '_') {
          value.push_back(src[iter]);
          iter++;
          col++;
        }

        if (value == "null") {
          kind = Literal;
          lit_kind = Null;
          return Token(kind, meta, value, lit_kind);
        }

        if (value == "true" || value == "false") {
          kind = Literal;
          lit_kind = Bool;
          return Token(kind, meta, value, lit_kind);
        }
        return Token(kind, meta, value);
      }
      else if (isdigit(chr)) {
        kind = Literal;
        lit_kind = Integer;

        while (isdigit(src[iter]) || src[iter] == '.') {
          if (src[iter] == '.' && lit_kind == Integer) {
            lit_kind = Float;
          }
          value.push_back(src[iter]);
          iter++;
          col++;
        }
        return Token(kind, meta, value, lit_kind);
      }
      panic("unresolved sequence: " + std::string(1, chr), meta);
      break;
  }
  iter++;
  col++;

  if (!lit_kind && value.empty()) {
    return Token(kind, meta);
  }

  if (!lit_kind) {
    return Token(kind, meta, value);
  }

  return Token(kind, meta, value, lit_kind);

}

const char Tokenizer::peek() const {
  return iter + 1 < len ? src[iter + 1] : '\0';
}

const char Tokenizer::peek_two() const {
  return iter + 2 < len ? src[iter + 2] : '\0';
}

const char Tokenizer::peek_three() const {
  return iter + 3 < len ? src[iter + 3] : '\0';
}

inline const std::string Tokenizer::to_str() {
  return src;
}

inline const bool Tokenizer::is_newl() {
  return prev == '\n';
}

inline const bool Tokenizer::is_eof() {
  return prev == '\0';
}

std::string Token::to_str() {
  switch (kind)
  {
    case LineComment: return "LineComment: " + value;
    case BlockComment: return "BlockComment: " + value;
    case Identifier: return "Identifier: " + value;
    case Literal: return "Literal: " + value;
    case Eof: return "Eof";
    case Slash: return "Slash";
    case Sub: return "Sub";
    case Arrow: return "Arrow";
    case Dot: return "Dot";
    case Eq: return "Eq";
    case NotEq: return "NotEq";
    case FatArrow: return "FatArrow";
    case EqEq: return "EqEq";
    case OpenBrace: return "OpenBrace";
    case CloseBrace: return "CloseBrace";
    case OpenParen: return "OpenParen";
    case CloseParen: return "CloseParen";
    case OpenBracket: return "OpenBracket";
    case CloseBracket: return "CloseBracket";
    case Comma: return "Comma";
    case Semi: return "Semi";
    case Colon: return "Colon";
    case Add: return "Add";
    case Star: return "Star";
    case At: return "At";
    case Hash: return "Hash";
    case Not: return "Not";
    case LessThan: return "LessThan";
    case GreaterThan: return "GreaterThan";
    case AndAnd: return "AndAnd";
    case OrOr: return "OrOr";
    case AddEq: return "AddEq";
    case SubEq: return "SubEq";
    case StarEq: return "StarEq";
    case SlashEq: return "SlashEq";
    case LessThanEq: return "LessThanEq";
    case GreaterThanEq: return "GreaterThanEq";
    default: return "Unknown";
  }
}
