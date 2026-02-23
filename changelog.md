# Changelog

## General
- Add structure initializers `{ ... }`.
- Add namespaces `... :: space { ... }`.
- Add namespace specifiers `...::...`
- Remove the `mut` keyword and qualified types.

## lace
- Add new `StructInitExpr` for struct initializer expressions.
- Add new `SpaceDefn` for namespace definitions.
- Add new `Specifier` to `RefExpr` for namespace specifiers.
- Update parser and tests to use new `TokenStream` construct.
- Removed `mut` qualifier and type mutability contracts.
- Simplified type system to use raw instances of `Type`, and removed `QualType`.
- Add automatic (STL) linkiing via `-l` and `-stl/-no-stl` flags.
- Move `AST::Context` out into `AST` itself, and update all uses.
- Replace uses of "loaded" with "imports".

## LIR
- Updated SSA rewrite pass to new LIR definitions.
- Allocate space on the stack for call arguments where applicable.
