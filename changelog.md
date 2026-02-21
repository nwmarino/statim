# Changelog

## General

## lace
- Add new `StructInitExpr` for struct initializer expressions.
- Update parser and tests to use new `TokenStream` construct.
- Removed `mut` qualifier and type mutability contracts.
- Simplified type system to use raw instances of `Type`, and removed `QualType`.
- Add automatic (STL) linkiing via `-l` and `-stl/-no-stl` flags.
- Move `AST::Context` out into `AST` itself, and update all uses.
- Replace uses of "loaded" with "imports".

## LIR
- Updated SSA rewrite pass to new LIR definitions.
