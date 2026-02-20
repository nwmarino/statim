//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

//
//  This source file defines a small arena allocator fur dynamically allocating
//  formatted strings at runtime.
//
//  clang -ffreestanding -fno-stack-protector -fno-builtin -nostdlib -O2 -c
//

extern void* mem_alloc(unsigned long);
extern void* mem_realloc(void*, unsigned long);
extern void mem_free(void*);

typedef struct FmtArena {
    unsigned char* base;
    unsigned long capacity;
    unsigned long offset;
} FmtArena;

static unsigned long align_up(unsigned long value, unsigned long align) {
    return (value + align - 1) & ~(align - 1);
}

void __fmt_arena_init(FmtArena* arena, unsigned long capacity) {
    arena->base = (unsigned char*) mem_alloc(capacity);
    arena->capacity = capacity;
    arena->offset = 0;
}

void __fmt_arena_destroy(FmtArena* arena) {
    mem_free(arena->base);
    arena->base = 0;
    arena->capacity = 0;
    arena->offset = 0;
}

void __fmt_arena_reset(FmtArena* arena) {
    arena->offset = 0;
}

void* __fmt_arena_alloc(FmtArena* arena, unsigned long size, unsigned long align) {
    unsigned long aligned_offset = align_up(arena->offset, align);
    unsigned long new_offset = aligned_offset + size;

    if (new_offset > arena->capacity) {
        unsigned long new_capacity = arena->capacity * 2;
        if (new_capacity < new_offset)
            new_capacity = new_offset;

        unsigned char* new_base = 
            (unsigned char*) mem_realloc(arena->base, new_capacity);

        arena->base = new_base;
        arena->capacity = new_capacity;
    }

    void* p = arena->base + aligned_offset;
    arena->offset = new_offset;
    return p;
}
