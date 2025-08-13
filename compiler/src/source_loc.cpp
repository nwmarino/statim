#include "input_file.hpp"
#include "source_loc.hpp"
#include "logger.hpp"

using namespace stm;

static std::string source(const Span& span) {
    static thread_local std::string src;
    
    const std::string& full = span.begin.file.source();

    std::size_t start_pos = 0;
    std::size_t end_pos = full.length();
    bool start_found = false;

    std::size_t line = 1;
    std::size_t col = 1;

    for (std::size_t idx = 0; idx != full.length(); ++idx) {
        if (!start_found && line == span.begin.line && col == span.begin.column) {
            start_pos = idx;
            start_found = true;
        }

        if (line == span.end.line && col == span.end.column) {
            end_pos = idx + 1;
            break;
        }

        if (full[idx] == '\n') {
            line++;
            col = 1;
        } else {
            col++;
        }
    }

    src = full.substr(start_pos, end_pos - start_pos);
    return src;
}

void Span::info(const std::string& msg) const {
    logger_info(msg + '\n' + source(*this));
}

void Span::warn(const std::string& msg) const {
    logger_warn(msg + '\n' + source(*this));
}

void Span::error(const std::string& msg) const {
    logger_error(msg + '\n' + source(*this));
}

void Span::fatal(const std::string& msg) const {
    logger_fatal(msg + '\n' + source(*this));
}
