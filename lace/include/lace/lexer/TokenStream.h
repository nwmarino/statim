//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LACE_TOKEN_STREAM_H_
#define LACE_TOKEN_STREAM_H_

#include "lace/core/Common.h"
#include "lace/lexer/Token.h"

#include <algorithm>
#include <cassert>
#include <vector>

namespace lace {

class TokenStream final {
    std::vector<Token> m_tokens = {};
    uint64_t m_position = 0;

public:
    TokenStream() = default;

    ~TokenStream() = default;

    TokenStream(const TokenStream&) = delete;
    void operator=(const TokenStream&) = delete;

    TokenStream(TokenStream&&) noexcept = delete;
    void operator=(TokenStream&&) noexcept = delete;

    /// Push the given |token| onto this stream.
    void push(const Token& token) {
        m_tokens.push_back(token);
    }

    /// Returns the current token in the stream.
    [[nodiscard]] const Token& get() const {
        assert(m_position < size() && "index out of bounds!");
        return m_tokens[m_position];
    }

    /// Returns the current position of this stream.
    [[nodiscard]] uint64_t position() const { return m_position; }

    /// Advance this stream by |n| positions.
    inline void advance(uint64_t n = 1) {
        // Keep the cursor between 0 and the end of the stream.
        m_position = std::clamp(m_position + n, 0ul, size() - 1ul);
    }

    /// Rewind this stream by |n| positions.
    inline void rewind(uint64_t n = 1) {
        advance(-n);
    }

    /// Seek to the absolute |position| of this stream.
    /// Fails if the position would exceed the bounds of this stream.
    [[nodiscard]] Result seek(uint64_t position) {
        if (position < 0 || position >= size())
            return false;

        m_position = position;
        return true;
    }

    /// Reset the position of this stream.
    void reset() {
        m_position = 0;
    }

    /// Test if this stream is complete, i.e. the end has been reached.
    [[nodiscard]] bool complete() const { 
        return m_position + 1 >= m_tokens.size(); 
    }

    /// Returns the size of this stream based on how many tokens are in it.
    uint64_t size() const { return m_tokens.size(); }
};

} // namespace lace

#endif // LACE_TOKEN_STREAM_H_
