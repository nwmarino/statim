# statim ('stat·​im) *"immediately"*

Statim is a multi-purpose, strongly typed language built with kernel 
development in mind. The main point of the project is to expirement with the 
potential of an interpretable intermediate represenation (IR) closely coupled 
into the compiler.

## Overview

Overarching priorities are reduced compilation times from that of modern C/C++, 
metaprogramming capabilities, and compile-time executions of arbitrary code. 
Although costly to the former, having a unique IR allows for some interesting 
compiler feedback and CTFE opportunities.

It's probably important to note that most of the concepts being implemented 
here aren't necessarily new or unique to this project, but ultimately serve as 
a tool for future projects.

### Long-term objectives

* Primitive build system using language constructs so nothing is needed other 
than the compiler itself

* Compile-time function & expression evaluation

* Syntax tree modifications

## Desugaring

Although keeping with the general purpose idea, to write software that we want, 
we need features like complex casting, pointer arithmetic, etc. and thus the 
language won't shy away from those constructs; it's not meant to be high-level.

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

### Structures

Structs will exist, but in a fashion much similar to that found in C.

```
Box :: {
// public by default
    length: u32,
    width: u32,
    height: u32,

$private
    contents: mut *void,
};
```

Only fields exist in the initial declaration. Methods won't be defined in the
traditional sense, but can be achieved by designating a receiver in a function,
i.e. for the `Box` type,

```
foo :: (self: mut *Box) -> i32 {
    ...
}
```

will allow for both `foo(x)` and `x.foo()`, but only the former if `x` has
type `*Box`.

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
`>`, `>=`, `[]`,

### Templates

Templates (more similar to those found in C++, not generics) via 
monomorphization will eventually exist, but come at a later date, probably 
after bootstrapping.

### Namespacing & Multiple Files

The language will not make use of a preprocessor or header files. To use 
multiple source files in a program, the `$use` rune can be utilized to import
the public declarations (and thereby types possibly) of a relative file:

```
$use "Utils.stm"; // imports all public symbols
```

To define an actual namespace - that forces mangling on all declarations within
it - one can nest `namespace ... { ... }` declarations as needed, and redeclare
the namespace wherever convenient.

Namespaces can be "scoped" into using the path `::` operator.

## Runes

Runes are a language construct that give way to much of the planned 
metaprogramming and decorative abilities (things like safety knobs, privacy 
modifiers, etc.) in the language.

### `$abi(...)`

Marks the following function as using a given ABI/language calling convention, 
i.e. `$abi("C++")`.

### `$abort`

Aborts the current process.

### `$alignas`

Specify the alignment for a struct member.

### `$asm`

Marks the beginning of an inline assembly block.

### `$assert`

A regular assert statement, configurably compiled.

### `$comptime`

The `$comptime` rune evaluates to true if the code is being executed at compile 
time, i.e.

```
if $comptime {
    // compile-time execution
} else {
    // ...
}
```

### `$deprecated`

Mark a function or type as deprecated, presenting a configurable warning or 
error to the user.

### `$if`

Compile-time if statement.

### `$inline`

Suggest (not force) the compiler to inline a function at callsites.

### `$intrinsic`

Marks a named declaration as being specifically handled by the compiler. Has no 
value on unrecognized code, i.e. that outside of the standard library.

### `$no_discard`

Enforce the usage of a function return value, can be configured to only present 
a warning or crash the compilation process on discard.

### `$no_optimize`

Mark a function as not to be optimized at any point during compilation.

### `$no_return`

Mark a function as non-returning.

### `$no_scope`

Make it so that the following list of statements enclosed by braces {, } does 
not define a new scope.

### `$packed`

Specify for a struct that no padding should be added to its members.

### `$path`

Evaluates to the path of the current file as a string.

### `$public`

Marks a declaration as public to any file that imports the one which the marked 
declaration is in.

### `$private`

Marks a declaration as private to the file it is defined in. By default, any 
and all declarations are private.

### `$print`

Print a formatted string of varying arguments to stdout. At compile-time, gets 
expanded to multiple print calls based on the types of the provided arguments.

### `$println`

Same as `$print`, but with an automatic newline.

### `$static_assert`

A compile-time assert statement.

### `$unsafe`

Used to mark a block of code as bypassing potential warnings and security 
features.

### `$write`

Similar to `$print`, but to a provided file.

### `$writeln`

Same as `$write`, but with an automatic newline.
