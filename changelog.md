# Changelog

## General
- Remove LLVM backend.
- Extend some standard library components e.g. runtime, strings, files.
- Remove some old statim code.
- Fixed bugs in runtime assembly.

## lace
- Fixed some formatting bugs in the diagnostics logger.
- Add phase timing to verbose outputs.
- Updated code generation for new LIR definitions.

## LIR
- Redesigned instruction API to use separate classes per instruction type.
- Added back the `Phi` node.
- Added the `extract` instruction for constant index field access of aggregates.
- Split the `pwalk` instruction into separate `access` and `index` instructions
for addressed structure access and pointer arithmetic, respectively.
- Move `Function::Arg` to a separate `Parameter` class.
- Add weak patch for non-i64 indices in pointer arithmetic instructions to sign 
extend into full x64 registers.
- Update old machine printer pass.
- Add multiple result types to functions.
- Remove 1-bit integer type `i1`, replaced with instances of `i8` where 
applicable e.g. comparisons.
- Replace `Machine` type methods with bit results instead of bytes.
