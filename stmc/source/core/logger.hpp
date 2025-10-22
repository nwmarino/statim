#ifndef STATIM_LOGGER_HPP_
#define STATIM_LOGGER_HPP_

#include "types/source_location.hpp"
#include "types/types.hpp"

#include <iostream>
#include <string>

namespace stm {

class Logger final {
    static std::ostream*    pOutput;
    static bool             color;

    static void log_src(const Span& span);

public:
    enum class Severity : u8 {
        Info, Warning, Fatal,
    };

    Logger() = delete;

    static void init(std::ostream& output = std::cerr);

    static void log(Severity severity, const std::string& msg);
    static void log(Severity severity, const std::string& msg, const Span& span);

    static void info(const std::string& msg);
    static void info(const std::string& msg, const Span& span);

    static void warn(const std::string& msg);
    static void warn(const std::string& msg, const Span &span);

    __attribute__((noreturn))
    static void fatal(const std::string& msg);

    __attribute__((noreturn))
    static void fatal(const std::string& msg, const Span &span);
};

} // namespace stm

#endif // STATIM_LOGGER_HPP_
