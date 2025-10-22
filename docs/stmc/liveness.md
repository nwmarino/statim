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

These constraints can be realized during instruction selection, with clear
flags on the physical registers used to help the allocator recognize where
these live ranges exist. Take for example the following,

```s
foo:
    ...
    mul %rbx            #    4| %rax is implicitly defined here
    movq %rax, -8(%rbp) #    5| %rax is used, but also killed (never used again for the mul result)
    ...                 # 6-11|
    movq $1, %rax       #   12| %rax is defined again here, for different reasons
    ret
```

to ensure that `%rax` is not considered live over the range 6-11, it needs to
be considered killed at 5 so that its live range gets split at the next
definition (see 12). This is one of many scenarios which supports the need
for context on physical register operands.

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

## Kill

The `kill` flag marks the death of a value, ending its live range.

If the value in `%rbx` used in `addq %rbx, %rax` is not used after the 
instruction, it is considered killed at that point.

## Dead 

The `dead` flag marks the result of a `def` as unused.

If `%rax` in `movq %rbx, %rax` is not used after the instruction, it is 
considered dead.

* *It is not considered a kill because its a definition, and a kill implies the 
existence of a range.*

## Extra

* Every register operand must be atleast a `use` or a `def`. Even if an operand 
kills a value, it has still technically used it.

* The `implicit` flag acts as both a modifier to the operand visibility as well
as the explicitness of its `use` or `def` flag. This means we can have
operands on instructions that don't appear in the final assembly. This is
important because it means we can describe all the effects an instruction has
without some extra descriptor info, and nicely dumps during print passes.
