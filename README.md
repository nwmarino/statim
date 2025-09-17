# statim ('stat·​im) *"immediately"*

Statim is to be a multi-purpose, strongly typed language designed with kernel 
development in mind. The main point of the project is to expirement with the 
potential of an interpretable intermediate represenation (IR) closely coupled 
into the compiler.

## Overview

Overarching priorities of the language are reduced compilation times from that 
of modern C/C++, metaprogramming capabilities, and compile-time executions of arbitrary code. Although costly to the former, having a unique IR allows for 
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

Only fields exist in the initial declaration. Methods won't be defined in the
traditional sense, but can be achieved by designating a receiver in a function,
i.e. for the `Box` type,

```
foo :: (self: mut *Box) -> s32 {
    ...
}
```

will allow for both `foo(x)` (if `x: *Box`) and `x.foo()`.

This means that with the all familiar method call syntax `.<name>()`, we can 
also add methods to built-in types:

```
to_string :: (self: *i64) -> string {
    ...
}
```

One important note is that since the methods are defined outside of the
structure, they have be in scope in order to be used. This means that apart
from traditional visiblity via `$public` and `$private`, certain methods can
be hidden if they are defined in a scope different from the parent scope of the
receiving struct.

### Operator Overloading

Operator overloading will be implemented by way the `operator ...` syntax, i.e.

```
operator + :: (self: Box, other: Box) -> Box {
    ...
}
```

Not all operators can be overriden, but the planned exceptions are:

`+`, `+=`, `-`, `-=`, `*`, `*=`, `/`, `/=`, `%`, `%=`, `==`, `!=`, `<`, `<=`, 
`>`, `>=`, `<<`, `>>`, `<<=`, `>>=`, `[]`, `[]=`

The reasoning behind not being able to override the assignment `=` operator
exists for the reason that it introduces a lot of ambiguity with copying.
Operator overloading already hides a lot of the "whats going on", and
the semantics of custom copy operators goes beyond the scope of the language. 

### Templates

Templates via monomorphization will eventually exist, but come at a later date, likely after bootstrapping.

### Namespacing & Multiple Files

The language will not make use of a preprocessor or header files. To use 
multiple source files in a program, the `use` keyword can be used to import
the public declarations of a relative file:

```c
use "Utils.stm";
```

To define an actual namespace - one that forces name mangling on all 
declarations within it - one can use the `namespace ... { ... }` declaration
which nests all the top-level declarations within it.

Namespaces can be stacked as needed, and are redeclarable wherever convenient.

Namespaces can be "scoped" into using the path `::` operator.
