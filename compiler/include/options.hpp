#ifndef STATIM_OPTIONS_HPP_
#define STATIM_OPTIONS_HPP_

#include "types.hpp"

namespace stm {

/// Different backend options.
enum class Backend : u8 {
    LLVM,
    X86_64,
};

/// Different operating system options.
enum class OpSys : u8 {
    Linux,
    Windows,
};

/// Different architecture options.
enum class Arch : u8 {
    X86_64,
};

/// Potential options and diagnostics for the compiler.
struct Options final {
    Backend     backend;
    OpSys       os;
    Arch        arch;
    const char* pOutput;
    u8          debug:1;
    u8          devel:1;
    u8          emit_asm:1;
    u8          keep_obj:1;
    u8          time:1;
};

} // namespace stm

#endif // STATIM_OPTIONS_HPP_
