# Runes

Runes are a language construct that give way to much of the planned 
metaprogramming and decorative abilities (things like safety knobs, privacy 
modifiers, etc.) in the language.

## Current Runes

### `$abort`

Aborts the current process.

### `$assert`

A regular, run-time assert statement.

### `$deprecated`

Mark a function or type as deprecated, presenting a configurable warning or 
error to the user.

### `$intrinsic`

Marks a named declaration as being specifically handled by the compiler. Has no 
value on unrecognized code, i.e. that outside of the standard library.

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

### `$write`

Similar to `$print`, but to a provided file.

### `$writeln`

Same as `$write`, but with an automatic newline.
