"Validate" pass on AST, filling in types, validating symbol table entries, parameter limits,
breaking points in loops, returns in functions, etc.

^ Also check for entry point main and that its prototype is valid with the standard.

- Check that a type exists within the scope of a declaration.
  - Check that the rval is of the same type.



Then, lower to IR (control flow graph) which is comprised of basic blocks, locals (memory locations on the
stack), and expressions which can be evaluated.

Basics are that for any function,

fn ident(a: i32) -> i32 {
  ...
},

it is divided into the following representation,

fn @ident(a: i32) -> i32 {
  let _1: i32;
  let _2; i32;
  ...                          # declare proto vars, conditional vars, etc.


  bb0: {
    ...
  }

  bb1: {
    ...
  }

  bb2: {
    ...
  }

  ...                           # represent body with basic blocks, each with one entry and one terminator


  bbz: {
    ...                         # final block with return
  }

}

This IR is based on a control flow graph and can easily allow for code generation since it is to a similar
level of abstract as most relevant asm sets we'll be compiling to (be it in phase 0 or 1-3).