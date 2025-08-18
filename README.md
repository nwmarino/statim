# statim ('stat·​im) *"immediately"*

Statim is a general-purpose, strongly typed language built with kernel development in mind. The 
main point of the project is to expirement with the capabilities of a bytecode closely coupled into 
the compiler itself.

## Overview

Overarching priorities are reduced compilation times from that of modern C/C++, 
metaprogramming capabilities, and compile-time executions of arbitrary code. Although costly to the 
former, having a unique bytecode allows for some interesting compiler feedback and compile-time 
opportunities. Since backing the bytecode with LLVM allows for getting things running faster,
keeping the bytecode high-level enough so as to not "walk backwards" during code generation is
an important design consideration.

### Long-term objectives

* Primitive build system using language constructs so nothing is needed other than the compiler itself

* Partial compile-time evaluation, i.e. loop & condition folding for known constants 

* Explicit type layout for factors like alignment, padding, partial members

* Representing physical registers as typed pointers

* Compile-time syntax tree modifications

* Faster compilation times than modern C++, ideally in the realm of 100K lines of code in half a 
second without caching

## Desugaring

Although keeping with the general purpose idea, to write the software that we often want, we need
features like complex casting, pointer arithmetic, etc. and thus the language won't shy away from
those constructs; it's not meant to be high-level.

### What *is* part of the plan

* compile-time execution of arbitrary code
* inline assembly
* operator overloading
* monomorphic templates
* parallelization
* dropping marked struct members
* runtime type reflection
* auto dereferencing (no `->` operator)
* functions with multiple return values
* file-based, user-controlled namespacing
* optional bounds, null pointer checks

### What *isn't* part of the plan

* function overloading (up in the air)
* constructors & destructors
* C++ like namespacing
* inheritance / virtual functions
* a preprocessor 
* external build system(s)
* built-in garbage collection
* RAII (in the sense of cleaning up entire objects)
* exceptions

### Inline Assembly

For now, inline assembly will follow the familiar GNU format using string literals, clobbers,
and potential side effect markers:

```
$asm {
    "mov $3, %rax",
    "add %rax, %rbx"
    : "~rax,~rbx,volatile"
};
```

### Operator Overloading

Operator overloading will be implemented by way of "magic" functions, similar to that in Python.
For example, to overload the `+` operator, one may implement the `__add__` method for the 
respective type.

### Templates

Templates (more similar to those found in C++, not generics) via monomorphization will eventually 
exist, but come at a later date, probably after bootsrapping.

### RAII / Member Destruction

RAII (Resource Acquisition is Initialization) won't be supported in the traditional sense by
tracking the lifetime of an entire object. Instead, particular members of a struct may be marked
with a tilde `~` to declare that the relevant destructing function should be called when the
owning object goes out of scope:

```
box :: {
    a: ~i64*,
    B: i64*,
}
```

This allows for full control of what parts of an object get destroyed, and no unexpected behaviour
with regards to objects unexpectedly being destroyed.

### Header files / Preprocessor

The language will not make use of a preprocessor or header files. To use multiple source files in
a program, the `use` keyword can be utilized to import the public declarations (and thereby types)
of a relative file, in one of three ways:

```
use "Utils.stm"; // import all public symbols

use { Foo, Bar } = "Utils.stm"; // import only Foo, Bar

use Utils = "Utils.stm"; // import everything under the namespace Utils
```

### Namespacing

Statim won't "declare" namespaces by way of `namespace ...`, instead files can be namespaced on
use, for example `use Utils = "utils.stm"`, can later by scoped into like `Utils::foo()`.

This model allows for only shallow namespacing while letting source code choose how to namespace
code in a clever way, relevant to the use case. 

### Structures and Methods

Structs will exist, but in a more "barebones" fashion, allowing for more control
over the layout and it's methods:

```
Box :: {
    length: u32,
    width: u32,
    height: u32,

    $[priv]
    contents: *void,
};
```

Only fields exist in the initial declaration. Methods can be amended later using
an `impl`:

```
impl Box {
    volume :: (self: *Box) -> u32 {
        ...
    }

    ...
}
```

This means that with the all familiar method call syntax `.()`, we can also
add methods to built-in types:

```
impl i64 {
    to_string :: (self: *i64) -> *char {
        ...
    }
}
```

Another important note is that since the implementation of methods are clearly
grouped, their name mangling can be clearly manipulated. For example, if methods
should be free floating, an impl can be decorated with `$mangle("free")` to
specify that. Methods only need a receiver `self` to be considered valid.

## Runes

Runes are a language construct that give way to much of the planned metaprogramming and
decorative abilities (things like safety knows, privacy modifiers, etc.) in the language.

### `$abi(...)`

Marks the following function as using a given ABI/language calling convention, i.e. `$abi("C++")`.

### `$abort`

Aborts the current process.

### `$alignas`

Specify the alignment for a struct member.

### `$asm`

Marks the beginning of an inline assembly block.

### `$assert`

A compile-time assert statement.

### `$code`

Specify a string to be inserted as code, i.e.

```
foo :: () i64 {
    $code "ret 0;"
}
```

### `$comptime`

The `$comptime` rune evaluates to true if the code is being executed at compile time, i.e.

```
if $comptime {
    // compile-time execution
} else {
    // ...
}
```

### `$deprecated`

Mark a function or type as deprecated, presenting a configurable warning or error to the user.

### `$dump`

Pretty-print the bytecode of a function at compile-time for potentially debugging purposes.

### `$if`

Compile-time if statement.

### `$inline`

Suggest (not force) the compiler to inline a function at callsites.

### `$intrinsic`

Marks a named declaration as being specifically handled by the compiler. Has no value on
unrecognized code, i.e. that outside of the standard library.

### `$no_discard`

Enforce the usage of a function return value, can be configured to only present a warning or
crash the compilation process on discard.

### `$no_optimize`

Mark a function as not to be optimized at any point during compilation.

### `$no_return`

Mark a function as non-returning.

### `$no_scope`

Make it so that the following list of statements enclosed by braces {, } does not define a new
scope.

### `$packed`

Specify for a struct that no padding should be added to its members.

### `$path`

Evaluates to the path of the current file as a string.

### `$public`

Marks a declaration as public to any file that imports the one which the marked declaration is in.

### `$private`

Marks a declaration as private to the file it is defined in. By default, any and all declarations
are private.

### `$print`

Print a formatted string of varying arguments to stdout. At compile-time, gets expanded to
multiple print calls based on the types of the provided arguments.

### `$println`

Same as `$print`, but with an automatic newline.

### `$unsafe`

Used to mark a block of code as bypassing potential warnings and security features.

### `$write`

Similar to `$print`, but to a provided file.

### `$writeln`

Same as `$write`, but with an automatic newline.

## Bytecode
