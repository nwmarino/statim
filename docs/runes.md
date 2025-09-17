# Runes

Runes are a language construct that give way to much of the planned 
metaprogramming and decorative abilities (things like safety knobs, privacy 
modifiers, etc.) in the language.

## Current Runes

### `$abi(...)`

Marks the following function as using a given ABI/language calling convention, 
i.e. `$abi("C++")`.

### `$abort`

Aborts the current process.

### `$alignas`

Specify the alignment for a struct member.

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

Suggest the compiler to inline a function at callsites.

### `$intrinsic`

Marks a named declaration as being specifically handled by the compiler. Has no 
value on unrecognized code, i.e. that outside of the standard library.

### `$no_discard`

Enforce the usage of a function return value, can be configured to only present 
a warning or crash the compilation process on discard.

### `$no_opt`

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
