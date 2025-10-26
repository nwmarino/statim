#ifndef SPBE_TYPE_H_
#define SPBE_TYPE_H_

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct SpbeCFG SpbeCFG;

typedef enum SpbeTypeKind {
    SPBE_TYPE_KIND_VOID,
    SPBE_TYPE_KIND_INT1,
    SPBE_TYPE_KIND_INT8,
    SPBE_TYPE_KIND_INT16,
    SPBE_TYPE_KIND_INT32,
    SPBE_TYPE_KIND_INT64,
    SPBE_TYPE_KIND_FLOAT32,
    SPBE_TYPE_KIND_FLOAT64,
    SPBE_TYPE_KIND_ARRAY,
    SPBE_TYPE_KIND_FUNCTION,
    SPBE_TYPE_KIND_POINTER,
    SPBE_TYPE_KIND_STRUCT,
} SpbeTypeKind;

typedef struct SpbeType {
    SpbeTypeKind        kind;
    u32                 id;
} SpbeType;

typedef struct SpbeArrayType {
    SpbeType            base;
    const SpbeType*     pElement;
    u32                 size;
} SpbeArrayType;

typedef struct SpbeFunctionType {
    SpbeType            base;
    u32                 num_args;
    const SpbeType**    ppArgs;
    const SpbeType*     pReturn;
} SpbeFunctionType;

typedef struct SpbePointerType {
    SpbeType            base;
    const SpbeType*     pPointee;
} SpbePointerType;

typedef struct SpbeStructType {
    SpbeType            base;
    char*               pName;
    u32                 num_fields;
    const SpbeType**    ppFields;
} SpbeStructType;

typedef struct SpbeStructTypeCreateInfo {
    const char*         pName;
    u32                 num_fields;
    const SpbeType**    ppFields;
} SpbeStructTypeCreateInfo;

SPBE_API u32
spbe_type_is_integer(const SpbeType* pType);

SPBE_API u32
spbe_type_is_integer_sized(const SpbeType* pType, u32 bytes);

SPBE_API u32
spbe_type_is_floating_point(const SpbeType* pType);

SPBE_API u32
spbe_type_is_floating_point_sized(const SpbeType* pType, u32 bytes);

SPBE_API SpbeType*
spbe_type_get_array_type(SpbeCFG* pCFG, const SpbeType* pElement, u32 size);

SPBE_API SpbeType*
spbe_type_get_function_type(SpbeCFG* pCFG, u32 num_args, 
                            const SpbeType** ppArgs, const SpbeType* pReturn);

SPBE_API SpbeType*
spbe_type_get_pointer_type(SpbeCFG* pCFG, const SpbeType* pPointee);

SPBE_API SpbeType*
spbe_type_get_struct_type(SpbeCFG* pCFG, const char* pName);

SPBE_API SpbeType*
spbe_type_create_struct_type(SpbeCFG* pCFG, 
                             const SpbeStructTypeCreateInfo* pInfo);

SPBE_API const SpbeType*
spbe_type_get_struct_field(const SpbeStructType* pType, u32 i);

SPBE_API void
spbe_type_add_field(SpbeStructType* pType, const SpbeType* pField);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SPBE_TYPE_H_
