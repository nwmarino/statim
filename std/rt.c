void __attribute__((noreturn)) exit(long code) {
#if defined(__x86_64__)
    asm volatile (
        "movq %0, %%rdi\n"
        "movq $60, %%rax\n"
        "syscall\n"
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

void __attribute__((naked)) __attribute((noreturn)) __abort(void) {
    asm volatile (
        "movq $62, %%rax\n"
        "movq $0, %%rdi\n"
        "movq $6, %%rsi\n"
        "syscall\n"
        ::: "rax", "rdi", "rsi"
    );
}

void __attribute((noreturn)) __panic(char* __s, unsigned long __n) {
    asm volatile (
        "movq $1, %%rax\n"
        "movq $2, %%rdi\n"
        "movq %0, %%rsi\n"
        "movq %1, %%rdx\n"
        "syscall\n"
        "call __abort\n"
        :
        : "r" (__s), "r" (__n)
        : "rax", "rdi", "rsi", "rdi"
    );

    __builtin_unreachable();
}

void __memcpy(void* __d, void* __s, unsigned long __n) {
    char* dst = (char*) __d;
    char* src = (char*) __s;

    while (__n > 0) {
        dst[__n] = src[__n];
        __n--;
    }
}

void __memset(void* __d, char __v, unsigned long __n) {
    char* dst = (char*) __d;

    while (__n > 0) {
        dst[__n] = __v;
        __n--;
    }
}

unsigned long __strlen(char* __s) {
    unsigned long n = 0;
    while (__s[n++] != '\0');
    return n;
}
