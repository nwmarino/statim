# Statim Interpretable Intermediate Representation (SIIR)

The SIIR of a program is it's semantically validated representation, ready for 
interpretation, codegen, and potential optimizations.

## Design Notes

Initially the middle-end IR was meant to be a bytecode, but it became pretty
apparent that such a low-level IR that could not easily take SSA form, and
thereby many modern optimization opportunities would be lost - which is a 
pretty big hit to the purpose of the compiler.
