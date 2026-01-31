//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LOVELACE_OPTIONS_H_
#define LOVELACE_OPTIONS_H_

//
//  This header file defines the Options structure, which contains various
//  options used by driver code to decide the behavior of the compilation
//  process.
//

#include <cstdint>
#include <string>

namespace lace {

struct Options final {
    /// The possible optimization levels.
    enum class OptLevel : uint32_t {
        Default,    //< (-0d) Only the necessary optimizations.
        Aggressive, //< (-0a) More aggressive optimizations. 
        Space,      //< (-0s) Optimizations for minimizing binary size.
    };

    std::string output; //< (-o) The name of the output file.
    OptLevel opt;       //< (-Od/-Oa/-Os) The optimization level.
    uint32_t threads;   //< (-j) Number of threads to use. 

    bool debug;         //< (-g) If debugging symbols should be added.
    bool multithread;   //< (-st) If multithreading should be used.
    bool verbose;       //< (-b) If extra notes should be logged.
    bool version;       //< (-v) If the version should be printed.

    bool dump_ast;     //< (-dump-ast) If the AST should be printed.
    bool dump_lir;     //< (-dump-lir) If the LIR should be printed.
    bool dump_mir;     //< (-dump-mir) If the MIR should be printed.
};

} // namespace lace

#endif // LOVELACE_OPTIONS_H_
