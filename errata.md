- Should not be able to dereference *, subscript [], access . or otherwise on a
void pointer.
- Allow unknown pointer types when two definitions across files depend on each
other.
- Should not be able to reference local variables before their definition.
- The SSA rewrite leads to have constant propogation throughout many of the
samples. This leads to alot of immediates and memory operands in the assembly,
which tend to violate architecture rules.
