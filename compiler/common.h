#ifndef SKVOZ_COMMON_H_
#define SKVOZ_COMMON_H_

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef unsigned char   u8;
typedef unsigned short  u16;
typedef unsigned int    u32;
typedef unsigned long   u64;

typedef signed char     i8;
typedef signed short    i16;
typedef signed int      i32;
typedef signed long     i64;

typedef float           f32;
typedef double          f64;

typedef enum : i8 {
    SK_FALSE = 0x0,
    SK_TRUE = 0x1,
} SkBool8;

typedef enum {
    SK_SUCCESS = 0x0,
    SK_FAILURE_UNKNOWN = 0x1,
    SK_FAILURE_OUT_OF_MEMORY = 0x2,
    SK_FAILURE_BAD_HANDLE = 0x3,
} SkResult;

#endif // SKVOZ_COMMON_H_
