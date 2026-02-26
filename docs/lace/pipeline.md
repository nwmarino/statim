**Lexical Analysis.**

Lex instances of `Token` into a `TokenStream` via the `Lexer` interface.

**Syntax Analysis.**

Parse each `TokenStream` into a `Rib` via the `Parser` interface.

**Rib Analysis.**

*pass I: `SymbolAnalysis`*

- Construct `Scope` trees that contain instances of `Symbol`.

*pass IIa: `NameResolution`*

- Shallowly traverse the topologically sorted packages, "completing" top-level
definitions and their type signatures.
  - Must be sequential, and in topological order of package dependencies.

*pass IIb: `SemanticAnalysis`*

- Deeply traverse the packages, resolving names and types of nested values, and
semantically validating the representation.
  - Resolve symbols using the pre-established scope trees. If an external 
    symbol is used, but it's respective package isn't used and/or it is private,
    then resolution fails.
  - Can be parallelized, since top-level symbols become read-only.

*pass III: `Codegen`*

- Traverse the representation, generating LIR code into a singular `CFG`.
  - Since we use one compilation unit, this is generally sequential but deep
    bodies can be parallelized.

(optional) **SSA Rewrite.**

The `CFG` may optionally be rewritten into a truer Static Single-Assignment 
form (SSA), replacing instances of the `Load` and `Store` instructions to/from 
`Local`s with `Phi` nodes; replacing memory accesses with values conditional 
upon control flow.

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

(optional) **MachIR Analysis.**

Optional target-specific optimizations can be performed, like peephole opts,
redundancy elimination, etc.

**Assembly Emission.**

Finally, the `AsmWriter` interface can emit plaintext assembly from the MachIR
`MachineObject`.
