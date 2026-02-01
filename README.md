# lovelace

lovelace is an imperative language meant for systems that takes major 
inspiration from the philosophies of your favorite langs. It is, however, most 
similar in nature to C by way of what is possible out of the box.

The project is split down the middle, with an x86-64 backend theoretically 
modular enough to be carved out and made to work with other frontends. 

### lace

lace is the frontend for the language, and in particular, handles the process
of turning source code into a "valid" syntax tree, which is used to represent
input programs. Later, a code generation pass turns the syntax tree into a 
target agnostic intermediate representation (LIR).

### LIR

The lovelace intermediate representation (LIR) handles target-specific jobs 
like ABI control, register allocation, and SSA-based optimizations. The IR is 
capable of true SSA form through an optional rewrite pass based on algorithms 
described by [Braun et al.](https://link.springer.com/chapter/10.1007/978-3-642-37051-9_6)
Since the IR is based on a control-flow graph, it can cleanly translate into 
similarly structured representation like LLVM IR.

## Building

All components of the project use and suggest at least CMake 4.0 to build.

Both the frontend and backend depend only on Boost and Google Test, which are
available on most distro package managers via `boost` and `gtest`.

Most of the compiler is written in C++20, with the main features used to 
justify it being ranges, the jthreads interface, and format strings.

Since the language only supports linux, there is little to no point building
binaries for windows.

```sh
cd lovelace/
cmake -S . -B build/
cmake --build build/

# for tests, optionally
ctest --test-dir build/
```
