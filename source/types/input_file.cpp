#include "core/logger.hpp"
#include "types/input_file.hpp"

#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/operations.hpp>

#include <fstream>
#include <ios>

using namespace stm;

const std::string& InputFile::filename() {
    if (!m_name.empty())
        return m_name;
    
    // Parse the filename if there are any path bits.
    m_name = path;
    auto pos = m_name.find_last_of("/\\");
    if (pos == std::string::npos)
        return m_name;

    return (m_name = m_name.substr(pos + 1));
}

const std::string& InputFile::absolute() {
    if (!m_absolute.empty())
        return m_absolute;

    return (m_absolute = boost::filesystem::absolute(path).string());
}

const std::string& InputFile::source() {
    if (!m_source.empty())
        return m_source;

    // Get the m_absoluteute path for the provided file.
    std::string absolute;
    try {
        absolute = boost::filesystem::canonical(path).string();
    } catch (const boost::filesystem::filesystem_error& err) {
        Logger::fatal("file does not exist: '" + std::string(path) + "'");
    }

    try {
        // Read in the contents of the file to the source string.
        auto size = boost::filesystem::file_size(path);
        m_source.resize(size);

        std::ifstream file(path, std::ios::binary);
        if (!file)
            Logger::fatal("failed to opeh source file: '" + std::string(path) + "'");

        file.read(m_source.data(), size);
        if (file.gcount() != static_cast<std::streamsize>(size))
            Logger::fatal("failed to read source file: '" + std::string(path) + "'");
    } catch (const std::exception& e) {
        Logger::fatal("failed to parse source file: '" + std::string(path) + "'");
    }

    return m_source;
}
