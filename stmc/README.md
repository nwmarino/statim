# statim ('stat·​im) *"immediately"*

The statim language is to be a multi-purpose, strongly typed language designed
with kernel development in mind. The main point of the project is to expirement 
with the potential of an interpretable intermediate represenation (IR) closely 
coupled into the compiler.

## Overview

Overarching priorities of the language are reduced compilation times from that 
of modern C/C++, metaprogramming capabilities, and compile-time executions of 
arbitrary code. Although costly to the former, having a unique IR allows for 
some interesting compiler feedback and compile-time opportunities.

## Desugaring

Although keeping with the general purpose idea, to write software that we want, 
we need features like complex casting, pointer arithmetic, etc. and thus the 
language won't shy away from those constructs; it's not meant to be 
"high-level".

### What *is* part of the plan

* compile-time evaluation
* operator overloading
* templates via monomorphization
* parallelization
* runtime type reflection
* auto dereferencing (no `->` operator)
* functions with multiple return values, natively
* namespaces
* `defer` statements
* optional bounds, null pointer checks

### What *isn't* part of the plan

* constructors, destructors
* `new` & `delete` operators
* garbage collector
* RAII
* function overloading
* inheritance
* a preprocessor 
* external build system(s)
* exceptions
* references

### Structs

Structs will exist, but in a fashion much similar to that found in C.

```
Box :: struct {
// public by default
    length: u32,
    width: u32,
    height: u32,

private:
    contents: mut *void,
};
```

### Using Multiple Source Files

The language will not make use of a preprocessor or header files. To use 
multiple source files in a program, the `use` keyword can be used to import
the public declarations of a relative file:

```c
use "Utils.stm";
```
