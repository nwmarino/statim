- Should not be able to dereference *, subscript [], access . or otherwise on a
void pointer.
- Allow unknown pointer types when two definitions across files depend on each
other.
- Should not be able to reference local variables before their definition.
- Phi's suffer from the "swap" problem.
