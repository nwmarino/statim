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
    callq   index.main@PLT
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

# stl.linux.exit :: (s64) -> void
    .text
    .global stl.linux.exit
    .type   stl.linux.exit, @function
stl.linux.exit:
    pushq	%rbp
	movq	%rsp, %rbp
    movq    16(%rbp), %rdi
    movq    $60, %rax
    syscall
    ud2

# stl.linux.open :: (*char, s64, s64) -> s64
    .text
    .global stl.linux.open
    .type   stl.linux.open, @function
stl.linux.open: 
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

# stl.linux.close :: (s64) -> s64
    .text
    .global stl.linux.close
    .type   stl.linux.close, @function
stl.linux.close:
    pushq	%rbp
	movq	%rsp, %rbp
    movq    16(%rbp), %rdi
    movq    $3, %rax
    syscall
    movq	%rbp, %rsp
	popq	%rbp
    retq

# stl.linux.lseek :: (s64, s64, s64) -> s64
    .text
    .global stl.linux.lseek
    .type   stl.linux.lseek, @function
stl.linux.lseek:
    pushq   %rbp
    movq    %rsp, %rbp
    movq    16(%rbp), %rdi
    movq    24(%rbp), %rsi
    movq    32(%rbp), %rdx
    movq    $8, %rax
    syscall
    movq	%rbp, %rsp
	popq	%rbp
    retq

# stl.linux.read :: (s64, *char, s64) -> s64
    .text
    .global stl.linux.read
    .type   stl.linux.read, @function
stl.linux.read:
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

# stl.linux.write :: (s64, *mut char, s64) -> s64
    .text
    .global stl.linux.write
    .type   stl.linux.write, @function
stl.linux.write:
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

# stl.linux.brk :: (u64) -> *void
    .text
    .global stl.linux.brk
    .type   stl.linux.brk, @function
stl.linux.brk:
    pushq	%rbp
	movq	%rsp, %rbp
    movq    16(%rbp), %rdi
    movq    $12, %rax
    syscall
    movq	%rbp, %rsp
	popq	%rbp
    retq
