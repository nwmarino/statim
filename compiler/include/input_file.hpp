#ifndef STATIM_INPUT_FILE_HPP_
#define STATIM_INPUT_FILE_HPP_

#include <string>
#include <cstring>

namespace stm {

struct InputFile final {
    const char* pPath;

    bool operator == (const InputFile& other) const {
        return !std::strcmp(pPath, other.pPath);
    }

private:
    std::string name = "";
    std::string src = "";
    
public:
    /// Get the filename for this input file.
    const std::string& filename();

    /// Get the source code of this input file as a string.
    const std::string& source();
};

} // namespace stm

#endif // STATIM_INPUT_FILE_HPP_
