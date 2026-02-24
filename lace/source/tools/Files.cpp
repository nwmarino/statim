//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/core/Diagnostics.h"
#include "lace/tools/Files.h"

#include <cstdint>
#include <fstream>

using namespace lace;

bool lace::read_file(const std::string& path, std::string& contents) {
    std::ifstream file(path, std::ios::ate);
    if (!file || !file.is_open()) {
        log::error("failed to open file: " + path);
        return false;
    }

    uint32_t size = file.tellg();
    contents.resize(size);
    file.seekg(std::ios::beg);

    if (!file.read(contents.data(), contents.size())) {
        log::error("failed to read file: " + path);
        return false;
    }

    file.close();
    return true;
}

std::string lace::without_extension(const std::string& path) {
    std::size_t first = path.find_last_of('.');
    if (first == std::string::npos)
        return path;

    return path.substr(0, first);
}

std::string lace::with_assembly_extension(const std::string& path) {
    return without_extension(path) + ".s";
}

std::string lace::with_object_extension(const std::string& path) {
    return without_extension(path) + ".o";
}
