#ifndef STATIM_OPTIONS_HPP_
#define STATIM_OPTIONS_HPP_

#include "types.hpp"

namespace stm {

/// Different backend options.
enum Backend : u8 {
    BACKEND_LLVM,
    BACKEND_X86_64,
};

/// Different operating system options.
enum OpSys : u8 {
    OPSYS_LINUX,
};

/// Different architecture options.
enum Arch : u8 {
    ARCH_X86_64,
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
