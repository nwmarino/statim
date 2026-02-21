//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LACE_FILES_H_
#define LACE_FILES_H_

//
//  This header file declares some useful tooling functions related to file I/O.
//

#include <string>

namespace lace {

/// Read in the contents of the file at the given |path| to |contents.
/// Returns the result of the operation.
[[nodiscard]] bool readFile(const std::string& path, std::string& contents);

/// Returns the given |path| without its last file extension.
std::string withoutExtension(const std::string& path);

/// Returns the given |path| with only the `.s` file extension.
std::string withAssemblyExtension(const std::string& path);

/// Returns the givn |path| with only the `.o` file extension.
std::string withObjectExtension(const std::string& path);

} // namespace lace

#endif // LACE_FILES_H_
