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
- Prevent duplicate input files using absolute path.
- Change header file convention to always use `.h`.
- Added new `TokenStream` class to later separate concerns with respect 
to lexing at parse-time.

## LIR
- Redesigned instruction API to use separate classes per instruction type.
- Cleanup AMD64 lowering code into a dedicated IR pass.
- Updated machine-specific API to use clearer operand types with better 
indirection.
- Updated printer, register analysis, and ASM write passes for new machine API.
- Added back the `Phi` node.
- Added the `extract` instruction for constant index field access of aggregates.
- Split the `pwalk` instruction into separate `access` and `index` instructions
for addressed structure access and pointer arithmetic, respectively.
- Move `Function::Arg` to a separate `Parameter` class.
- Add weak patch for non-i64 indices in pointer arithmetic instructions to sign 
extend into full x64 registers.
- Update old machine printer pass.
- Remove 1-bit integer type `i1`, replaced with instances of `i8` where 
applicable e.g. comparisons.
- Replace `Machine` type methods with bit results instead of bytes, and update 
uses appropriately.
