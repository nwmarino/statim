#ifndef SK_TYPE_H_
#define SK_TYPE_H_

#include "common.h"

typedef enum {
    SK_TYPE_KIND_PRIMITIVE,
    SK_TYPE_KIND_DEFERRED,
    SK_TYPE_KIND_POINTER,
} SkTypeKind;

typedef struct {
    u32             tid;
    SkTypeKind      kind;
    char*           pName;
} SkType;

typedef enum {
    SK_PRIMITIVE_TYPE_KIND_BOOL,
    SK_PRIMITIVE_TYPE_KIND_CHAR,
    SK_PRIMITIVE_TYPE_KIND_INT8,
    SK_PRIMITIVE_TYPE_KIND_INT16,
    SK_PRIMITIVE_TYPE_KIND_INT32,
    SK_PRIMITIVE_TYPE_KIND_INT64,
    SK_PRIMITIVE_TYPE_KIND_UINT8,
    SK_PRIMITIVE_TYPE_KIND_UINT16,
    SK_PRIMITIVE_TYPE_KIND_UINT32,
    SK_PRIMITIVE_TYPE_KIND_UINT64,
    SK_PRIMITIVE_TYPE_KIND_FLOAT32,
    SK_PRIMITIVE_TYPE_KIND_FLOAT64,
} SkPrimitiveTypeKind;

typedef struct {
    SkType                  base;
    SkPrimitiveTypeKind     kind;
}* SkPrimitiveType;

SkResult skInitPrimitiveType(SkPrimitiveTypeKind kind, SkPrimitiveType* pType);
void skDestroyPrimitiveType(SkPrimitiveType* pType);

typedef struct {
    u32 pointerLevel;
} SkTypeProps;

typedef struct {
    SkType      base;
    SkTypeProps props;
}* SkDeferredType;

SkResult skInitDeferredType(char *pName, SkTypeProps props, SkDeferredType *pType);
void skDestroyDeferredType(SkDeferredType *pType);

typedef struct {
    SkType      base;
    SkType*     pPointee;
}* SkPointerType;

SkResult skInitPointerType(SkType *pPointee, SkPointerType* pType);
void skDestroyPointerType(SkPointerType* pType);

#endif // SK_TYPE_H_
