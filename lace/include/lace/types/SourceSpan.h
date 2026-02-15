//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LACE_SOURCE_SPAN_H_
#define LACE_SOURCE_SPAN_H_

//
//  This header file defines the SourceSpan type, used to represent a span of source code between
//  two locations. Each node in the abstract syntax tree receives an instance of this type.
//

#include "lace/types/SourceLocation.h"

namespace lace {

/// Represents a span of source code between two locations.
struct SourceSpan final {
    SourceLocation start, end;

    SourceSpan() = default;

    SourceSpan(SourceLocation loc) : start(loc), end(loc) {}

    SourceSpan(SourceLocation start, SourceLocation end) : start(start), end(end) {}

    SourceSpan(const SourceSpan& other) {
        start = other.start;
        end = other.end;
    }

    void operator=(const SourceSpan& other) {
        start = other.start;
        end = other.end;
    }

    bool operator==(const SourceSpan& other) const {
        return start == other.start && end == other.end;
    }

    bool operator<(const SourceSpan& other) const {
        return start < other.start && end < other.end;
    }

    bool operator>(const SourceSpan& other) const {
        return start > other.start && end > other.end;
    }
};

} // namespace lace

#endif // LACE_SOURCE_SPAN_H_
