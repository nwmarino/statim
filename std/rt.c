void __attribute__((noreturn)) exit(long code) {
#if defined(__x86_64__)
    asm volatile (
        "movq %0, %%rdi\n"
        "movq $60, %%rax\n"
        "syscall"
        :
        : "r" (code)
        : "rax", "rdi"
    );
#else
    #error "unsupported architecture"
#endif // defined(__x86_64__)
    
    __builtin_unreachable();
}

void __attribute__((naked)) _start(void) {
    asm volatile (
        "xorl %%ebp, %%ebp\n"
        "movq (%%rsp), %%rdi\n"
        "leaq 8(%%rsp), %%rsi\n"
        "call main\n"
        "movq %%rax, %%rdi\n"
        "movq $60, %%rax\n"
        "syscall\n"
        ::: "memory"
    );
}
