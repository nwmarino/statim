//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LACE_SOURCE_LOCATION_H_
#define LACE_SOURCE_LOCATION_H_

//
//  This header file defines the SourceLocation type, used in the frontend to track locations in 
//  source code.
//

#include <cstdint>

namespace lace {

struct SourceLocation {
    uint16_t line, col;

    SourceLocation(uint16_t line = 1, uint16_t col = 1) : line(line), col(col) {}

    SourceLocation(const SourceLocation& other) {
        line = other.line;
        col = other.col;
    }

    void operator=(const SourceLocation& other) {
        line = other.line;
        col = other.col;
    }

    bool operator==(const SourceLocation& other) const {
        return line == other.line && col == other.col;
    }

    bool operator<(const SourceLocation& other) const {
        return line < other.line && col < other.col;
    }

    bool operator>(const SourceLocation& other) const {
        return line > other.line && col > other.col;
    }
};

} // namespace lace

#endif // LACE_SOURCE_LOCATION_H_
