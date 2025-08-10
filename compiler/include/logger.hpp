#ifndef STATIM_LOGGER_HPP_
#define STATIM_LOGGER_HPP_

#include "types.hpp"
#include "source_loc.hpp"

#include <iostream>
#include <string>

namespace stm {

/// Different kinds of severity for logging functions.
enum LoggerSeverity : u8 {
    LOGGER_SEVERITY_INFO,
    LOGGER_SEVERITY_WARNING,
    LOGGER_SEVERITY_ERROR,
    LOGGER_SEVERITY_FATAL,
};

/// Initializer the logger with the given \p output stream.
void logger_init(std::ostream& output = std::cerr);

/// Log a \p msg at the specified \p severity.
void logger_log(LoggerSeverity severity, const std::string& msg, const SourceLocation* pLoc = nullptr);

/// Log an informative message \p msg with an optional source location \p pLoc.
void logger_info(const std::string& msg, const SourceLocation* pLoc = nullptr);

/// Log a warning error message \p msg with an optional source location \p pLoc.
void logger_warn(const std::string& msg, const SourceLocation* pLoc = nullptr);

/// Log a non-fatal error message \p msg, with an optional source location \p pLoc.
void logger_error(const std::string& msg, const SourceLocation* pLoc = nullptr);

/// Log a fatal error message \p msg, with an optional source location \p pLoc.
__attribute__((noreturn))
void logger_fatal(const std::string& msg, const SourceLocation* pLoc = nullptr);

} // namespace stm

#endif // STATIM_LOGGER_HPP_
