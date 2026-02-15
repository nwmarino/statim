//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LACE_DIAGNOSTICS_H_
#define LACE_DIAGNOSTICS_H_

//
//  This header file declares diagnostics tools for the compiler, namely a set of logging 
//  functions for the sake of informing the user about the behavior of their source code as it 
//  gets processed.
//

#include "lace/types/SourceLocation.h"
#include "lace/types/SourceSpan.h"

#include <iostream>
#include <ostream>
#include <string>

namespace lace {

namespace log {

/// A location in source suitable for the logger.
struct Location final {
    std::string path;
	uint32_t line, col;

    Location() = default;

    Location(const std::string& path, SourceLocation loc) : path(path), line(loc.line), col(loc.col) {}
};

/// A span of source suitable for the logger.
struct Span final {
    std::string path;
	SourceLocation start;
	SourceLocation end;

    Span() = default;

    Span(const std::string& path, SourceSpan span) : path(path), start(span.start), end(span.end) {}
};

/// Initialize the logger with the given output stream |os|.
///
/// If a custom stream is given, then it is to be borrowed and not owned.
void init(std::ostream& os = std::cerr);

/// Change the output stream of the logger to |os|.
void set_output_stream(std::ostream& os);

/// Clear the logger output stream, if there is one.
///
/// This effectively disables the logger until a new output stream is provided
/// via set_output_stream.
void clear_output_stream();

/// Flush the output stream and the compiler state if there have been any 
/// errors declared.
///
/// Effectively, if any non-fatal error calls were made, then flushing will crash the
/// compiler at the point of the call.
void flush();

/// Log the given |msg| as a note to the output stream.
void note(const std::string& msg);

/// Log the given |msg| as a note to the output stream, alongside the given
/// source |loc|.
void note(const std::string& msg, const Location& loc);

/// Log the given |msg| as a note to the output stream, alongside the given
/// source |span|.
void note(const std::string& msg, const Span& span);

/// Log the given |msg| as a warning to the output stream. 
void warn(const std::string& msg);

/// Log the given |msg| as a warning to the output stream, alongside the
/// given source |loc|.
void warn(const std::string& msg, const Location& loc);

/// Log the given |msg| as a warning to the output stream, alongside the
/// given source |span|.
void warn(const std::string& msg, const Span& span);

/// Log the given |msg| as an error to the output stream.
void error(const std::string& msg);

/// Log the given |msg| as an error to the output stream, alongside
/// the given source |loc|.
void error(const std::string& msg, const Location& loc);

/// Log the given |msg| as an error to the output stream, alongside
/// the given source |span|.
void error(const std::string& msg, const Span& span);

/// Log the given |msg| as a fatal error to the output stream.
[[noreturn]] void fatal(const std::string& msg);

/// Log the given |msg| as a fatal error to the output stream, alongside
/// the given source |loc|.
[[noreturn]] void fatal(const std::string& msg, const Location& loc);

/// Log the given |msg| as a fatal error to the output stream, alongside
/// the given source |span|.
[[noreturn]] void fatal(const std::string& msg, const Span& span);

} // namespace log

} // namespace lace

#endif // LACE_DIAGNOSTICS_H_
