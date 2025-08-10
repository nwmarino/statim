#include "../include/input_file.hpp"
#include "../include/logger.hpp"

#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/operations.hpp>

#include <fstream>
#include <ios>

using namespace stm;

const std::string& InputFile::filename() {
    if (!name.empty())
        return name;
    
    // Parse the filename if there are any path bits.
    name = pPath;
    auto pos = name.find_last_of("/\\");
    if (pos == std::string::npos)
        return name;

    return (name = name.substr(pos + 1));
}

const std::string& InputFile::source() {
    if (!src.empty())
        return src;

    // Get the absolute path for the provided file.
    std::string absolute_path;
    try {
        absolute_path = boost::filesystem::canonical(pPath).string();
    } catch (const boost::filesystem::filesystem_error& err) {
        logger_fatal("file does not exist: " + std::string(pPath));
    }

    try {
        // Read in the contents of the file to the src string.
        u32 size = boost::filesystem::file_size(pPath);
        src.resize(size);

        std::ifstream file(pPath, std::ios::binary);
        if (!file)
            logger_fatal("failed to open file for reading: " + std::string(pPath));

        file.read(src.data(), size);
        if (file.gcount() != static_cast<std::streamsize>(size))
            logger_fatal("failed to read entire source file: " + std::string(pPath));
    } catch (const std::exception& e) {
        logger_fatal("failed to parse input file: " + std::string(pPath));
    }

    return src;
}
