#ifndef SPBE_CONSTANT_H_
#define SPBE_CONSTANT_H_

#include "common.h"
#include "instruction.h"
#include "value.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef enum SpbeConstantKind {
    SPBE_CONSTANT_KIND_CONSTANT_INT,
    SPBE_CONSTANT_KIND_CONSTANT_FP,
    SPBE_CONSTANT_KIND_CONSTANT_NULL,
    SPBE_CONSTANT_KIND_CONSTANT_STRING,
    SPBE_CONSTANT_KIND_BLOCK_ADDRESS,
    SPBE_CONSTANT_KIND_GLOBAL,
} SpbeConstantKind;

typedef struct SpbeConstant {
    SpbeUser        base;
} SpbeConstant;

typedef struct SpbeConstantInt {
    SpbeConstant    base;
    i64             value;
} SpbeConstantInt;

typedef struct SpbeConstantFP {
    SpbeConstant    base;
    f64             value;
} SpbeConstantFP;

typedef struct SpbeConstantNull {
    SpbeConstant    base;
} SpbeConstantNull;

typedef struct SpbeConstantString {
    SpbeConstant    base;
    const char*     pValue;
} SpbeConstantString;

typedef struct SpbeBlockAddress {
    SpbeConstant    base;
    SpbeBasicBlock* pBlock;
} SpbeBlockAddress;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SPBE_CONSTANT_H_
