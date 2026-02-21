# Changelog

## General

## lace
- Add new `StructInitExpr` for struct initializer expressions.
- Update parser and tests to use new `TokenStream` construct.
- Removed `mut` qualifier and type mutability contracts.
- Simplified type system to use raw instances of `Type`, and removed `QualType`.

## LIR
- Updated SSA rewrite pass to new LIR definitions.
