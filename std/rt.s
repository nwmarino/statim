#   Copyright (c) 2025-2026 Nicholas Marino
#  
#   All runtime functions defined here assume the lace pure-stack ABI.

    .text
    .global _start
    .type   _start, @function
_start:
    callq   __rt_init
    callq   main@PLT
    movq    %rax, %rdi
    movq    $60, %rax   # exit syscall
    syscall
    ud2                 # unreachable

    .text
    .type   __rt_init, @function
__rt_init:
#   call    __fmt_arena_init@PLT
    retq

    .text
    .type   __rt_shutdown, @function
__rt_shutdown:
#   call    __fmt_arena_destroy@PLT
    retq

# __copy :: (*void, *void, s64) -> void
    .text
    .global __copy
    .type   __copy, @function
__copy:
    pushq	%rbp
	movq	%rsp, %rbp
    movq    16(%rbp), %rdi
    movq    24(%rbp), %rsi
    movq    32(%rbp), %rdx
.__copy_cnd:
    cmpq    $0, %rdx
    jne     .__copy_bdy
    movq	%rbp, %rsp
	popq	%rbp
    retq
.__copy_bdy:
    movb    (%rsi), %al
    movb    %al, (%rdi)
    incq    %rdi
    incq    %rsi
    decq    %rdx
    jmp     .__copy_cnd

# __abort :: () -> void
    .text
    .global __abort
    .type   __abort, @function
__abort:
    movq    $62, %rax
    movq    $0, %rdi    # pid 0
    movq    $0, %rsi    # signal 6 (SIGABRT)
    syscall             # kill syscall
    callq   __unreachable 

# __unreachable :: () -> void
    .text
    .global __unreachable
    .type   __unreachable, @function
__unreachable:
    ud2

# exit :: (s64) -> void
    .text
    .global exit
    .type   exit, @function
exit:
    movq    16(%rbp), %rdi
    movq    $60, %rax
    syscall

# open :: (*char, s64, s64) -> s64
    .text
    .global open
    .type   open, @function
open: 
    movq    16(%rbp), %rdi
    movq    24(%rbp), %rsi
    movq    32(%rbp), %rdx
    movq    $2, %rax
    syscall
    retq

# close :: (s64) -> s64
    .text
    .global close
    .type   close, @function
close:
    movq    16(%rbp), %rdi
    movq    $3, %rax
    syscall
    retq

# read :: (s64, *char, s64) -> s64
    .text
    .global read
    .type   read, @function
read:
    movq    16(%rbp), %rdi
    movq    24(%rbp), %rsi
    movq    32(%rbp), %rdx
    movq    $0, %rax
    syscall
    retq

# write :: (s64, *mut char, s64) -> s64
    .text
    .global write
    .type   write, @function
write:
    movq    16(%rbp), %rdi
    movq    24(%rbp), %rsi
    movq    32(%rbp), %rdx
    movq    $1, %rax
    syscall
    retq

# brk :: (u64) -> *void
    .text
    .global brk
    .type   brk, @function
brk:
    movq    16(%rbp), %rdi
    movq    $12, %rax
    syscall
    retq
