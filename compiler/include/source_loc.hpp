#ifndef STATIM_SOURCE_LOC_HPP_
#define STATIM_SOURCE_LOC_HPP_

#include "types.hpp"
#include <string>

namespace stm {

struct InputFile;

/// Represents a location in source code.
struct SourceLocation final {
    InputFile&  file;
    u32         line;
    u32         column;

    SourceLocation(InputFile& file, u32 line, u32 column) : file(file), line(line), column(column) {};

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

/// Represents the span of source between two locations in source.
struct Span final {
    SourceLocation begin;
    SourceLocation end;

    Span(const SourceLocation& loc) 
        : begin(loc), end(loc) {};

    Span(const SourceLocation& begin, const SourceLocation& end) 
        : begin(begin), end(end) {};

    bool operator == (const Span& other) const {
        return begin == other.begin && end == other.end;
    }

    bool operator < (const Span& other) const {
        return begin < other.begin && end < other.end;
    }

    bool operator > (const Span& other) const {
        return begin > other.begin && end > other.end;
    }

    void info(const std::string& msg) const;
    
    void warn(const std::string& msg) const;

    void error(const std::string& msg) const;

    void fatal(const std::string& msg) const;
};

} // namespace stm

#endif // SourceLocation
