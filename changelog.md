# Changelog

## General
- Add `rib` definitions.
- Revise `load` definitions to `use`s.
- Change compilation unit to `Rib`s, dependent on their root specifier, e.g. 
  `x` in `x::y`.
- Add default structure field initializers via `'=' <expr> ';'`.
- Add new file reading sample.
- Add line-by-line debugging symbols via the `-g` option.
- Add help option `-h`.

## lace
- Replace `AST` with `Rib` definitions.
- Revise `Scope` to use instances of `Symbol` for better name information.
- Update primary pipeline to use 3 deep passes:
  * `SymbolAnalysis` for scope tree constructions
  * `NameResolution` for name and type resolution
  * `SemanticAnalysis` for type checking and semantic rules
- Revise `LIRCodegen` pass to a new, `VisitorBase`-based `Codegen` pass that 
  uses querying and memoization for less unused code.
- Add null-terminators to `StringLiteral`s during code generation.
- Update `Codegen` to add debug locations based on command line options.

## LIR
- Add new `DebugNode` tree and `DebugBuilder`.
- Add new `AMD64Analysis` pass to remove redundant assembly.
- Add new `ConstantFolding` LIR pass to fold constants.
- Add new `TrivialDCEPass` LIR pass to remove trivially dead values.
- Use buffer size around callsites when spilling registers.
- Fix bug relating to string lowering (missing null-termination).
- Change how operands are lowered to AMD64 (valued, memory-addressible, address).
- Remove IR comments from assembly.
- Fix bug with phi node lowering.
