#
#   Copyright (c) 2025-2026 Nicholas Marino
#   All rights reserved.
#
#   All runtime functions defined here assume the lace pure-stack ABI.
#

    .text
    .global _start
    .type   _start, @function
_start:
    callq   main@PLT
    movq    %rax, %rdi
    movq    $60, %rax   # exit syscall
    syscall
    ud2                 # unreachable

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
    ud2                 # unreachable

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
    pushq	%rbp
	movq	%rsp, %rbp
    movq    16(%rbp), %rdi
    movq    $60, %rax
    syscall
    ud2

# open :: (*char, s64, s64) -> s64
    .text
    .global open
    .type   open, @function
open: 
    pushq	%rbp
	movq	%rsp, %rbp
    movq    16(%rbp), %rdi
    movq    24(%rbp), %rsi
    movq    32(%rbp), %rdx
    movq    $2, %rax
    syscall
    movq	%rbp, %rsp
	popq	%rbp
    retq

# close :: (s64) -> s64
    .text
    .global close
    .type   close, @function
close:
    pushq	%rbp
	movq	%rsp, %rbp
    movq    16(%rbp), %rdi
    movq    $3, %rax
    syscall
    movq	%rbp, %rsp
	popq	%rbp
    retq

# read :: (s64, *char, s64) -> s64
    .text
    .global read
    .type   read, @function
read:
    pushq	%rbp
	movq	%rsp, %rbp
    movq    16(%rbp), %rdi
    movq    24(%rbp), %rsi
    movq    32(%rbp), %rdx
    movq    $0, %rax
    syscall
    movq	%rbp, %rsp
	popq	%rbp
    retq

# write :: (s64, *mut char, s64) -> s64
    .text
    .global write
    .type   write, @function
write:
    pushq	%rbp
	movq	%rsp, %rbp
    movq    16(%rbp), %rdi
    movq    24(%rbp), %rsi
    movq    32(%rbp), %rdx
    movq    $1, %rax
    syscall
    movq	%rbp, %rsp
	popq	%rbp
    retq

# brk :: (u64) -> *void
    .text
    .global brk
    .type   brk, @function
brk:
    pushq	%rbp
	movq	%rsp, %rbp
    movq    16(%rbp), %rdi
    movq    $12, %rax
    syscall
    movq	%rbp, %rsp
	popq	%rbp
    retq
