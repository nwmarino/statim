#include "core/logger.hpp"
#include "types/input_file.hpp"

#include <cassert>
#include <cstdlib>
#include <vector>

using namespace stm;

static std::vector<std::string> source(const Span& span) {
    std::vector<std::string> lines { span.end.line - span.begin.line };
    const std::string& full = span.begin.file.source();

    std::size_t line = 1;
    std::size_t start = 0;
    for (std::size_t idx = 0; idx <= full.length(); ++idx) {
        if (idx == full.length() || full[idx] == '\n') {
            if (line >= span.begin.line && line <= span.end.line)
                lines.push_back(full.substr(start, idx - start));
            
            start = idx + 1;
            line++;
        }
    }

    return lines;
}

std::ostream* Logger::pOutput = nullptr;
bool Logger::color = false;

void Logger::log_src(const Span& span) {
    u32 line_len = std::to_string(span.begin.line).length();

    *pOutput << std::string(line_len + 2, ' ') << "┌─[" << 
        span.begin.file.absolute() << ':' << span.begin.line << "]\n";

    u32 line_n = span.begin.line;
    for (auto line : source(span)) {
        
        if (Logger::color)
            *pOutput << "\e[38;5;240m" << line_n++ << "\033[0m" << 
                std::string(line_len + 2 - std::to_string(line_n).length(), ' ') 
                    << "│ " << line << '\n';
        else
            *pOutput << line_n++ << ' ' << line << '\n';
    }

    *pOutput << std::string(line_len + 2, ' ') << "╰──\n";
}

void Logger::init(std::ostream& output) {
    Logger::pOutput = &output;
    Logger::color = pOutput == &std::cout || pOutput == &std::cerr;
}

void Logger::log(Severity severity, const std::string& msg) {
    if (!pOutput) return;
    
    switch (severity) {
    case Severity::Info:
        return info(msg);
    case Severity::Warning:
        return warn(msg);
    case Severity::Fatal:
        return fatal(msg);
    }
}

void Logger::log(Severity severity, const std::string& msg, const Span& span) {
    if (!pOutput) return;

    switch (severity) {
    case Severity::Info:
        return info(msg, span);
    case Severity::Warning:
        return warn(msg, span);
    case Severity::Fatal:
        return fatal(msg, span);
    }
}

void Logger::info(const std::string& msg) {
    if (!pOutput) return;

    *pOutput << "stmc: ";
    
    if (Logger::color)
        *pOutput << "\033[1;35minfo:\033[0m ";
    else
        *pOutput << "info: ";

    *pOutput << msg << '\n';
}

void Logger::info(const std::string& msg, const Span& span) {
    if (!pOutput) return;

    if (Logger::color)
        *pOutput << "\033[1;35m !\033[0m ";
    else
        *pOutput << " ! ";

    const SourceLocation& begin = span.begin;
    InputFile& file = begin.file;
    *pOutput << msg << '\n';
    log_src(span);
}

void Logger::warn(const std::string& msg) {
    if (!pOutput) return;

    *pOutput << "stmc: ";
    
    if (Logger::color)
        *pOutput << "\033[1;33mwarning:\033[0m ";
    else
        *pOutput << "warning: ";

    *pOutput << msg << '\n';
}

void Logger::warn(const std::string& msg, const Span &span) {
    if (!pOutput) return;

    if (Logger::color)
        *pOutput << "\033[1;33m ⚠︎\033[0m ";
    else
        *pOutput << " ⚠︎ ";

    const SourceLocation& begin = span.begin;
    InputFile& file = begin.file;
    *pOutput << msg << '\n';
    log_src(span);
}

__attribute__((noreturn))
void Logger::fatal(const std::string& msg) {
    if (pOutput) {
        *pOutput << "stmc: ";
        
        if (Logger::color)
            *pOutput << "\033[1;31mfatal:\033[0m ";
        else
            *pOutput << "fatal: ";

        *pOutput << msg << std::endl;
    }

    std::exit(EXIT_FAILURE);
}

__attribute__((noreturn))
void Logger::fatal(const std::string& msg, const Span &span) {
    if (pOutput) {
        if (Logger::color)
            *pOutput << "\033[1;31m ˣ\033[0m ";
        else
            *pOutput << " ˣ ";
        
        const SourceLocation& begin = span.begin;
        InputFile& file = begin.file;
        *pOutput << msg << '\n';
        log_src(span);
    }

    std::exit(EXIT_FAILURE);
}
