//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LACE_LEXER_H_
#define LACE_LEXER_H_

//
//  This header file declares the Lexer class, whom instances thereof interpret strings of source 
//  code into tokens usable by the parser for syntax analysis.
//

#include "lace/core/Common.h"
#include "lace/lexer/Token.h"

#include <cassert>
#include <string>

namespace lace {

class Lexer final {
    std::string m_source;
    std::string m_filename;
    uint32_t m_cursor = 0;
    SourceLocation m_loc = {};

    /// Returns the character the cursor is currently looking at.
    /// 
    /// If the end of the source buffer has been reached i.e. there is no
    /// character to look at, then the null terminator is returned instead.
    inline char curr() const { return isEof() ? '\0' : m_source[m_cursor]; }

    /// Returns the character |n| positions ahead in the source code buffer.
    /// 
    /// If |n| exceeds the size of the source buffer, then the null terminator
    /// is returned instead.
    inline char peek(uint32_t n = 1) const {
        return (m_cursor + n < m_source.size()) ? m_source[m_cursor + n] : '\0';
    }

    /// Move the lexer cursor |n| positions forward, and update the location in
    /// source accordingly.
    inline void move(uint32_t n = 1) {
        m_cursor += n;
        m_loc.col += n;
    }

    /// Update the location of the lexer per a new line.
    inline void endLine() {
        m_loc.line++;
        m_loc.col = 1;
    }

public:
    /// Create a new lexer using the given |source| buffer.
    ///
    /// Optionally, the |filename| argument designates the source file which |source| is from, and 
    /// allows for more accurate diagnostics should there be unrecognized tokens. 
    Lexer(const std::string& source, const std::string& filename = "") : m_source(source), 
                                                                         m_filename(filename) {}

    /// Test if the end of the source code buffer has been reached.
    Result isEof() const { return m_cursor >= m_source.size(); }

    /// Lex a new token and save its state to |token|.
    void lex(Token& token);
};

} // namespace lace

#endif // LACE_LEXER_H_
