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

## Runes

Runes are a language construct that give way to much of the planned metaprogramming and
decorative abilities (things like safety knows, privacy modifiers, etc.) in the language.

### `$asm`

## Bytecode
