**Lexical Analysis.**

Lex instances of `Token` into a `TokenStream` via the `Lexer` interface.

**Syntax Analysis.**

Parse each `TokenStream` into a `Rib` via the `Parser` interface.

**Rib Analysis.**

*pass I: `SymbolAnalysis`*

- Construct `Scope` trees that contain instances of `Symbol`.

*pass IIa: `NameResolution`*

- Resolve names and deferred types of nodes with only scope information, and 
no type information.

*pass IIb: `SemanticAnalysis`*

- Finish any name resolution which may depend on type information, and 
semantically validate the syntax tree.

*pass III: `Codegen`*

- Traverse the representation, generating LIR code into a `CFG` for each 
compilation unit, or top-level rib.

(optional) **SSA Rewrite.**

The `CFG` may optionally be rewritten into a "truer" Static Single-Assignment 
form (SSA), replacing instances of the `Load` and `Store` instructions to/from 
`Local`s with `Phi` nodes. In other words, replacing memory accesses with 
values conditional upon control flow.

**Lowering.**

Target-specific pass (e.g. `AMD64LoweringPass`) that generates a specialized 
assembly representation called MachIR into a `MachineObject` from a largely 
target-agnostic `CFG`.

**Register Analysis.**

The MachIR is linearly scanned via `LinearScan`, and instances of 
`LiveRange` are constructed from positions of register liveness. The live 
ranges are then assigned physical `Register`s via the `RegisterAllocator`
interface.
 
Subsequently, callsites require the ABI-designated caller-saved registers to be 
spilled to the stack, and this analysis is conducted by the `CallsiteAnalysis` 
pass.

(optional) **Architecture Analysis.**

Optional target-specific optimizations can be performed, like peephole opts,
redundancy elimination, etc.

**Assembly Emission.**

Finally, the `AsmWriter` interface can emit plaintext assembly from the MachIR
`MachineObject`.
