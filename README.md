# lace

Lace is an imperative systems language meant that takes major inspiration from 
the language philosophies of old. It is, however, most similar in nature to C 
by way of what is possible right now.

The project is modularized about the lace frontend and an AMD64 backend 
isolated enough to be carved out and made to work with other frontends in the
future.

### Lace Intermediate Representation (LIR)

The LIR handles target-specific jobs like ABI control, register allocation, and 
SSA-based optimizations. The IR is capable of SSA form through an optional 
rewrite pass based on algorithms described by 
[Braun et al.](https://link.springer.com/chapter/10.1007/978-3-642-37051-9_6)
Since the IR is based on a control-flow graph, it can theoretically translate 
into similar representations like LLVM IR.

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

## Usage

The minimal runtime and STL only support modern Linux and x86_64 assembly.

The runtime library, which maintains the `_start` routine and some other 
important functionality should be assembled first:

```sh
cd stl/
as rt.s -o rt.o
```
