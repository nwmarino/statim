#ifndef STATIM_OPTIONS_H_
#define STATIM_OPTIONS_H_

#include "types.hpp"

namespace stm {

/// Potential options and diagnostics for the compiler.
struct Options final {
    const char* output;
    u8 opt_level;
    u8 debug:1;
    u8 devel:1;
    u8 dump_ast:1;
    u8 dump_llvm_ir:1;
    u8 dump_machine_ir:1;
    u8 dump_siir:1;
    u8 keep_asm:1;
    u8 keep_obj:1;
    u8 link:1;
    u8 llvm:1;
    u8 nostd:1;
    u8 time:1;
};

} // namespace stm

#endif // STATIM_OPTIONS_H_
