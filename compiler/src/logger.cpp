#include "../include/logger.hpp"

#include <cassert>

using namespace stm;

static std::ostream* pOutput = nullptr;

void stm::logger_init(std::ostream& output) {
    pOutput = &output;
}

void stm::logger_log(LoggerSeverity severity, const std::string& msg, const SourceLocation* pLoc) {
    if (!pOutput)
        return;

    switch (severity) {
    case LOGGER_SEVERITY_INFO:
        return logger_info(msg, pLoc);
    case LOGGER_SEVERITY_WARNING:
        return logger_warn(msg, pLoc);
    case LOGGER_SEVERITY_ERROR:
        return logger_error(msg, pLoc);
    case LOGGER_SEVERITY_FATAL:
        return logger_fatal(msg, pLoc);
    }

    assert(false && "unknown logger severity kind");
}

void stm::logger_info(const std::string& msg, const SourceLocation* pLoc) {
    if (!pOutput)
        return;

    if (pLoc)
        *pOutput << pLoc->pFile << ':' << pLoc->line << ':' << pLoc->column << ": ";
    else
        *pOutput << "stmc: ";

    *pOutput << "info: " << msg << '\n';
}

void stm::logger_warn(const std::string& msg, const SourceLocation* pLoc) {
    if (!pOutput)
        return;

    if (pLoc)
        *pOutput << pLoc->pFile << ':' << pLoc->line << ':' << pLoc->column << ": ";
    else
        *pOutput << "stmc: ";

    *pOutput << "warning: " << msg << '\n';
}

void stm::logger_error(const std::string& msg, const SourceLocation* pLoc) {
    if (!pOutput)
        return;

    if (pLoc)
        *pOutput << pLoc->pFile << ':' << pLoc->line << ':' << pLoc->column << ": ";
    else
        *pOutput << "stmc: ";

    *pOutput << "error: " << msg << '\n';
}

__attribute__((noreturn))
void stm::logger_fatal(const std::string& msg, const SourceLocation* pLoc) {
    if (pOutput) {
        if (pLoc)
            *pOutput << pLoc->pFile << ':' << pLoc->line << ':' << pLoc->column << ": ";
        else
            *pOutput << "stmc: ";

        *pOutput << "fatal: " << msg << std::endl;
    }

    std::exit(EXIT_FAILURE);
}
