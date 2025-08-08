#ifndef STATIM_TYPE_H_
#define STATIM_TYPE_H_

#include "core.h"
#include "scope.h"

#ifdef __cplusplus
    extern "C" {
#endif // __cplusplus

typedef struct StmType_T* StmType;
typedef struct StmPrimitiveType_T* StmPrimitiveType;
typedef struct StmDeferredType_T* StmDeferredType;
typedef struct StmPointerType_T* StmPointerType;

/// Different kinds of types.
typedef enum StmTypeKind {
    STM_TYPE_KIND_PRIMITIVE,
    STM_TYPE_KIND_DEFERRED,
    STM_TYPE_KIND_POINTER,
} StmTypeKind;

/// Different kinds of primitive types.
typedef enum StmPrimitiveTypeKind {
    STM_PRIMITIVE_TYPE_KIND_BOOL,
    STM_PRIMITIVE_TYPE_KIND_CHAR,
    STM_PRIMITIVE_TYPE_KIND_SINT8,
    STM_PRIMITIVE_TYPE_KIND_SINT16,
    STM_PRIMITIVE_TYPE_KIND_SINT32,
    STM_PRIMITIVE_TYPE_KIND_SINT64,
    STM_PRIMITIVE_TYPE_KIND_UINT8,
    STM_PRIMITIVE_TYPE_KIND_UINT16,
    STM_PRIMITIVE_TYPE_KIND_UINT32,
    STM_PRIMITIVE_TYPE_KIND_UINT64,
    STM_PRIMITIVE_TYPE_KIND_FLOAT32,
    STM_PRIMITIVE_TYPE_KIND_FLOAT64,
} StmPrimitiveTypeKind;

/// Context for deferred type resolution.
typedef struct StmTypeContext {
    u32         pointerLevel;
    StmScope    scope;
} StmTypeContext;

STM_API_ATTR StmResult STM_API_CALL skInitPrimitiveType(StmPrimitiveTypeKind kind, StmPrimitiveType* pType);
STM_API_ATTR void STM_API_CALL skDestroyPrimitiveType(StmPrimitiveType* pType);

STM_API_ATTR StmResult STM_API_CALL skInitDeferredType(StmTypeContext* pContext, StmDeferredType* pType);
STM_API_ATTR void STM_API_CALL skDestroyDeferredType(StmDeferredType* pType);

STM_API_ATTR StmResult STM_API_CALL skInitPointerType(StmType pointee, StmPointerType* pType);
STM_API_ATTR void STM_API_CALL skDestroyPointerType(StmPointerType* pType);

#ifdef __cplusplus
    }
#endif // __cplusplus

#endif // STATIM_TYPE_H_
