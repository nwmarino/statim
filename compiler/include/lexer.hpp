#ifndef STATIM_LEXER_HPP_
#define STATIM_LEXER_HPP_

#include "input_file.hpp"
#include "token.hpp"

#include <vector>

namespace stm {

class Lexer final {
    InputFile&          mFile;
    std::string         mBuf;
    std::vector<Token>  mLexed;
    SourceLocation      mLoc;
    u32                 mPos = 0;

public:
    Lexer(InputFile& file, const std::string& src = "");

    /// Get the most previously lexed token.
    const Token& last() const;

    /// Get the token lexed \p n iterations ago.
    const Token& last(u32 n) const;

    /// Lex a new token.
    const Token& lex();

private:
    /// Test if the end of the buffer has been reached.
    bool is_eof() const;

    /// Get the current character in the stream.
    char curr() const;

    /// Peek at the character in \p n positions.
    char peek(u32 n = 1) const;

    /// End the current source location line.
    void end_line();

    /// Move the lexer cursor by \p n positions.
    void move(u32 n = 1);
};

} // namespace stm

#endif // STATIM_LEXER_HPP_
