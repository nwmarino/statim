// clang -ffreestanding -fno-stack-protector -fno-builtin -nostdlib -c rt.c

void __attribute__((noreturn)) exit(long code) {
    asm volatile (
        "movq %0, %%rdi\n"
        "movq $60, %%rax\n"
        "syscall\n"
        :
        : "r" (code)
        : "rax", "rdi"
    );
    
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

void __print(char* __s) {
    unsigned long bytes = __strlen(__s);
    asm volatile (
        "movq $1, %%rax\n"
        "movq $1, %%rdi\n"
        "movq %0, %%rsi\n"
        "movq %1, %%rdx\n"
        "syscall\n"
        :
        : "r" (__s), "r" (bytes)
        : "rax", "rdi", "rsi", "rdi"
    );
}

void __err(char* __s) {
    unsigned long bytes = __strlen(__s);
    asm volatile (
        "movq $1, %%rax\n"
        "movq $2, %%rdi\n"
        "movq %0, %%rsi\n"
        "movq %1, %%rdx\n"
        "syscall\n"
        :
        : "r" (__s), "r" (bytes)
        : "rax", "rdi", "rsi", "rdi"
    );
}

void __print_fd(long __fd, char* __s) {
    unsigned long bytes = __strlen(__s);
    asm volatile (
        "movq $1, %%rax\n"
        "movq %0, %%rdi\n"
        "movq %1, %%rsi\n"
        "movq %2, %%rdx\n"
        "syscall\n"
        :
        : "r" (__fd), "r" (__s), "r" (bytes)
        : "rax", "rdi", "rsi", "rdi"
    );
}

void __print_bool(long __fd, signed char __v) {
    if (__v) {
        __print_fd(__fd, (char*) "true");
    } else {
        __print_fd(__fd, (char*) "false");
    }
}

void __print_char(long __fd, char __c) {
    char buf[2];
    buf[0] = __c;
    buf[1] = '\0';
    __print_fd(__fd, buf);
}

void __print_si(long __fd, long __v, long __b) {
    char buf[65];
    long i = 0;
    char neg = 0;

    if (__v == 0)
        return __print_fd(__fd, (char*) "0");
    
    if (__v < 0) {
        neg = 1;
        __v = -__v;
    }

    while (__v != 0) {
        long rem = __v % __b;
        if (rem > 9) {
            buf[i++] = (rem - 10) + 'a';
        } else {
            buf[i++] = rem + '0';
        }

        __v = __v / __b;
    }

    if (neg == 1)
        buf[i++] = '-';
    
    buf[i] = '\0';

    long start = 0;
    long end = i - 1;
    while (start < end) {
        const char tmp = buf[start];
        buf[start] = buf[end];
        buf[end] = tmp;
        end--;
        start++;
    }

    __print_fd(__fd, buf);
}

void __print_ui(long __fd, unsigned long __v, long __b) {
    char buf[65];
    long i = 0;

    if (__v == 0)
        return __print_fd(__fd, (char*) "0");
    
    while (__v != 0) {
        unsigned long rem = __v % ((unsigned long)(__b));
        if (rem > 9) {
            buf[i++] = (rem - 10) + 'a';
        } else {
            buf[i++] = rem + '0';
        }

        __v = __v / ((unsigned long)(__b));
    }

    buf[i] = '\0';

    long start = 0;
    long end = i - 1;
    while (start < end) {
        const char tmp = buf[start];
        buf[start] = buf[end];
        buf[end] = tmp;
        end--;
        start++;
    }

    __print_fd(__fd, buf);
}

void __print_float(long __fd, float __v) {
    int iprt = (int) __v;
    float fprt = (__v - ((float) iprt)) * 1000000;

    __print_si(__fd, iprt, 10);
    __print_fd(__fd, (char*) ".");
    __print_si(__fd, (long) fprt, 10);
}

void __print_double(long __fd, double __v) {
    long iprt = (long) __v;
    double fprt = (__v - ((double) iprt)) * 1000000;

    __print_si(__fd, iprt, 10);
    __print_fd(__fd, (char*) ".");
    __print_si(__fd, (long) fprt, 10);
}

void __print_ptr(long __fd, void* __p) {
    __print_fd(__fd, (char*) "0x");
    __print_si(__fd, (long) __p, 10);
}
