#ifndef STATIM_INPUT_FILE_HPP_
#define STATIM_INPUT_FILE_HPP_

#include <string>
#include <cstring>

namespace stm {

class Span;

/// An input file given to the compiler.
struct InputFile final {
    const char* path;

    bool operator == (const InputFile& other) const {
        return std::strcmp(path, other.path) == 0;
    }

private:
    std::string m_name = "";
    std::string m_absolute = "";
    std::string m_source = "";
    
public:
    /// Get the filename for this input file.
    const std::string& filename();

    /// Get the absolute path for this input file.
    const std::string& absolute();

    /// Get the source code of this input file as a string.
    const std::string& source();

    /// Get the source code of this input file between two locations.
    const std::string& source(const Span& span);

    /// Overwrite the source of this input file, for devel purposes.
    void overwrite(const std::string& source) { m_source = source; }
};

} // namespace stm

#endif // STATIM_INPUT_FILE_HPP_
