#ifndef STATIM_CORE_H_
#define STATIM_CORE_H_

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#if defined(_WIN32)
    #define STM_API_ATTR __declspec(dllexport)
    #define STM_API_CALL __stdcall
#else
    #define STM_API_ATTR __attribute__((visibility("default")))
    #define STM_API_CALL
#endif // defined(_WIN32)

#ifdef __cplusplus
    extern "C" {
#endif // __cplusplus

typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long i64;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long u64;
typedef float f32;
typedef double f64;

/// Single byte boolean representation.
typedef enum StmBool8 : i8 {
    STM_FALSE = 0x0,
    STM_TRUE = 0x1,
} StmBool8;

/// Possible result codes from API functions.
typedef enum StmResult : i32 {
    STM_SUCCESS = 0x0,
    STM_FAILURE_UNKNOWN,
    STM_FAILURE_OUT_OF_MEMORY,
} StmResult;

typedef struct StmArray_T* StmArray;

STM_API_ATTR StmResult STM_API_CALL stmInitArray(u32 capacity, StmArray* pArray);
STM_API_ATTR void STM_API_CALL stmDestroyArray(StmArray* pArray);
STM_API_ATTR StmResult STM_API_CALL stmArrayPush(StmArray array, const void* pElement);
STM_API_ATTR const void* STM_API_CALL stmArrayPop(StmArray array);
STM_API_ATTR const void* STM_API_CALL stmArrayGet(StmArray array, u32 index);
STM_API_ATTR const void** STM_API_CALL stmArrayGetData(StmArray array);
STM_API_ATTR u32 STM_API_CALL stmArrayGetSize(StmArray array);
STM_API_ATTR u32 STM_API_CALL stmArrayGetapacity(StmArray array);
STM_API_ATTR StmResult STM_API_CALL stmArrayReserve(StmArray array, u32 n);

#ifdef __cplusplus
    }
#endif // __cplusplus

#endif // STATIM_CORE_H_
