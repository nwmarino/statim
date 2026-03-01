# Changelog

## General
- Add `rib` definitions.
- Revise `load` definitions to `use`s.
- Change compilation unit to `Rib`s, dependent on their root specifier, e.g. 
  `x` in `x::y`.

## lace
- Replace `AST` with `Rib` definitions.
- Revise `Scope` to use instances of `Symbol` for better name information.
- Update primary pipeline to use 3 deep passes:
  * `SymbolAnalysis` for scope tree constructions
  * `NameResolution` for name and type resolution
  * `SemanticAnalysis` for type checking and semantic rules
- Revise `LIRCodegen` pass to a new, `VisitorBase`-based `Codegen` pass that 
  uses querying and memoization for less unused code.

## LIR
