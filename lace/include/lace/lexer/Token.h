//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LACE_TOKEN_H_
#define LACE_TOKEN_H_

//
//  This header file defines the important token structure, which is a representation of the 
//  smallest piece of source code recognizable to the compiler.
//

#include "lace/types/SourceLocation.h"

#include <string>

namespace lace {

/// Represents a token lexed from source.
struct Token final {
    /// The different kinds of tokens.
    enum Kind {
		/// End of an input file.
        EndOfFile,
		/// `(`
        OpenParen,
		/// `)`
    	CloseParen,
		/// `{`
        OpenBrace,
		/// `}`
        CloseBrace,
		/// `[`
        OpenBrack,
		/// `]`
        CloseBrack,
		/// `=`
        Eq,
		/// `==`
        EqEq,
		/// `!`
        Bang,
		/// `!=`
        BangEq,
		/// `+`
        Plus,
		/// `-`
        Minus,
		/// `*`
        Star,
		/// `/`
        Slash,
		/// `%`
        Percent,
		/// `<`
        Left,
		/// `<=`
        LeftEq,
		/// `<<`
        LeftLeft,
		/// `>`
        Right,
		/// `>=`
        RightEq,
		/// `>>`
        RightRight,
		/// `&`
        And,
		/// `&&`
        AndAnd,
		/// `|`
        Or,
		/// `||`
        OrOr,
		/// `^`
        Xor,
		/// `->`
        Arrow,
		/// `.`
        Dot,
		/// `,`
        Comma,
		/// `:`
        Colon,
		/// `::`
        Path,
		/// `;`
        Semi,
        /// `~`
        Tilde,
		/// `$`
        Sign,
		/// identifiers, e.g. `ret` and `_x`
        Identifier,
		/// integers, e.g. `0` and `1337`
        Integer,
		/// floats, e.g. `1.` and `3.14`
        Float,
		/// characters, e.g. `'a'` and `'0'`
        Character,
		/// strings, e.g. `"hello"` and `"world"`
        String,
    } kind; //< The kind of token this is.

    /// The location of this token in source.
    SourceLocation loc;

    /// The attached value of this token for literals and identifiers. 
    std::string value;

    Token(Kind kind = Token::EndOfFile, SourceLocation loc = {}, const std::string& value = "")
      : kind(kind), loc(loc), value(value) {}

    bool operator==(const Token& other) const {
        return kind == other.kind && loc == other.loc && value == other.value;
    }

	/// Test if this token marks the end of an input file.
    inline bool isEof() const { return kind == Token::EndOfFile; }
};

} // namespace lace

#endif // LOVELACE_TOKEN_H_
