Each meaningful source file in lace begins with the definition of a unique rib:

```
// cli.lace

rib types::color;

Color :: enum {
    Red,
    Blue,
    Green,
}
```

These ribs make up the greater software, and essentially force namespacing upon
the codebase for very clear organization. And although we never defined the 
outer `types` rib, the compiler implicitly creates it, thus organizing the
compilation unit for `types` and all the ribs part of it.

If the software should have an entry point, or special build configurations,
then the recognized `index` rib should be defined in said file:

```
// index.lace

rib index;
```

In order to pull in all the public symbols a rib defines, the `use` keyword can
be used:

```
// index.lace

rib index;

use types::color;
```

This allows us to use the `Color` type from `index.lace` via `types::color::Color`.
Moreover, it should become clear that lace works not on the paths of files, but
by the ribs which the files alias themselves as.

However, using such a long specifier cuts down on readability in the codebase,
and the rib tree of `Color` can often be inferred in our project. With the `using`
keyword, we can flatten the scope:

```
// index.lace

rib index;

load types::color;

using types::color;
```

As such, we can now refer to `Color` without any specifiers. Doing so however, 
requires that the symbols being pulled in will not conflict name-wise with any 
existing symbols in scope.

