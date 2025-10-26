#ifndef SPBE_COMMON_H_
#define SPBE_COMMON_H_

#ifndef SPBE_API
    #if defined(_WIN32) && !defined(__CYGWIN__)
        #ifdef SPBE_BUILD
            #define SPBE_API __declspec(dllexport)
        #else
            #define SPBE_API __declspec(dllimport)
        #endif // SPBE_BUILD
    #else
        #define SPBE_API __attribute__((visibility("default")))
    #endif // defined(_WIN32) && !defined(__CYGWIN__)
#endif // SPBE_API

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long u64;

typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long i64;

typedef float f32;
typedef double f64;

#endif // SPBE_COMMON_H_
