# Register Liveness

## Remarks

This file contains some personal notes about register liveness for instruction 
selection and register allocation. Specifically, it's just some examples for
physical register operands and their flags.

## Example

Consider, during instruction selection, and regardless of the target 
architecture, some ABI rules or instructions may require the use of particular 
registers, i.e. division ops in x86_64 or the clobbering of caller-saved 
registers around callsites.

Only physical registers necessarily need special context, since they are not
uniquely numbered as in SSA. The allocator cannot discern the use of `%rax`
in the beginning and end of a function as it can for a virtual register `$42`.

These constraints can be realized during instruction selection, with clear
flags on the physical registers used to help the allocator recognize where
these live ranges exist. Take for example the following,

```s
foo:
    ...
    mulq %rbx           #    4| %rax is implicitly defined here
    movq %rax, -8(%rbp) #    5| %rax is used, but expires immediately (never used again for the mul result)
    ...                 # 6-11|
    movq $1, %rax       #   12| %rax is defined again here, for a different reason
    retq
```

to ensure that `%rax` is not considered live over the range 6-11, it needs to
be considered "expired" at 5 so that its live range gets split at the next
definition (see 12). This is one of many scenarios which supports the need for 
context on physical register operands.

## Def 

A `def` marks the beginning of a new value.

```
movq %rbx, %rax # explicitly defines %rax
```

### Implicit Def

An implicit `def` is one not outlined in the final assembly instruction. For 
example, the instruction

```
mul rbx  # rdx:rax = rax * rbx
```

explicitly uses `%rbx`, but it also implicitly defines `%rdx` and `%rax` to 
hold the result of the operation.

## Use 

A `use` marks the usage of a value, continuing its lifetime.

### Implicit Use

Similar to an implicit `def`, an implicit `use` makes use of a register not
appearing in the instruction. In the same example above,

```
mul rbx  # rdx:rax = rax * rbx
```

`%rax` is implicitly used as an operand to the operation.

## Expired

The `expired` flag on a used register indicates that the value in it will never
be used again, effectively ending its live range at that location.

As an example, the integer division instructions `idiv` and `div` perform 
`RDX:RAX / op` and store the quotient into `rax` and remainder into `%rdx`. 
This means that the instructions (implicitly) use `%rax` and `%rdx`, but also 
cause them to expire in the process.

## Extra

* Every register operand must be atleast a `use` or a `def`. Even if an register
immediately expires, it's value still technically existed.

* The `implicit` flag acts as both a modifier to the operand visibility as well
as the explicitness of its `use` or `def` flag. This means machine instructions
can carry pure context just in their operand list. This is important because 
it means we can describe all the effects an instruction has without some extra 
descriptor info, and nicely dumps during print passes.
