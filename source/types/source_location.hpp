#ifndef STATIM_SOURCE_LOCATION_HPP_
#define STATIM_SOURCE_LOCATION_HPP_

#include "types.hpp"

namespace stm {

struct InputFile;

/// A location in source code.
struct SourceLocation final {
    InputFile& file;
    u32 line;
    u32 column;

    SourceLocation(InputFile& file, u32 line, u32 column) 
        : file(file), line(line), column(column) {}

    bool operator == (const SourceLocation& other) const {
        return &file == &other.file && line == other.line 
          && column == other.column;
    }

    bool operator < (const SourceLocation& other) const {
        return line < other.line && column < other.column;
    }

    bool operator > (const SourceLocation& other) const {
        return line > other.line && column > other.column;
    }
};

/// A span of source code between two locations.
struct Span final {
    SourceLocation begin;
    SourceLocation end;

    Span(const Span&) = default;

    Span(const SourceLocation& loc) : begin(loc), end(loc) {}

    Span(const SourceLocation& begin, const SourceLocation& end) 
        : begin(begin), end(end) {}

    bool operator == (const Span& other) const {
        return begin == other.begin && end == other.end;
    }

    bool operator != (const Span& other) const {
        return begin != other.begin || end != other.end;
    }

    bool operator < (const Span& other) const {
        return begin < other.begin && end < other.end;
    }

    bool operator > (const Span& other) const {
        return begin > other.begin && end > other.end;
    }
};

} // namespace stm

#endif // STATIM_SOURCE_LOCATION_HPP_
