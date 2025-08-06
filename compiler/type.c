#include "type.h"
#include "common.h"

#include <assert.h>
#include <string.h>

static u32 tid = 0;

static const char* get_primitive_name(SkPrimitiveTypeKind kind) {
    switch (kind) {
    case SK_PRIMITIVE_TYPE_KIND_BOOL:
        return "bool";
    case SK_PRIMITIVE_TYPE_KIND_CHAR:
        return "char";
    case SK_PRIMITIVE_TYPE_KIND_INT8:
        return "i8";
    case SK_PRIMITIVE_TYPE_KIND_INT16:
        return "i16";
    case SK_PRIMITIVE_TYPE_KIND_INT32:
        return "i32";
    case SK_PRIMITIVE_TYPE_KIND_INT64:
        return "i64";
    case SK_PRIMITIVE_TYPE_KIND_UINT8:
        return "u8";
    case SK_PRIMITIVE_TYPE_KIND_UINT16:
        return "u16";
    case SK_PRIMITIVE_TYPE_KIND_UINT32:
        return "u32";
    case SK_PRIMITIVE_TYPE_KIND_UINT64:
        return "u64";
    case SK_PRIMITIVE_TYPE_KIND_FLOAT32:
        return "f32";
    case SK_PRIMITIVE_TYPE_KIND_FLOAT64:
        return "f64";
    }
}

SkResult skInitPrimitiveType(SkPrimitiveTypeKind kind, SkPrimitiveType* pType) {
    if (!pType)
        return SK_FAILURE_BAD_HANDLE;

    *pType = malloc(sizeof(**pType));
    if (!*pType)
        return SK_FAILURE_OUT_OF_MEMORY;

    SkType base;
    base.tid = tid++;
    base.kind = SK_TYPE_KIND_PRIMITIVE;
    base.pName = (char*) get_primitive_name(kind);

    (*pType)->base = base;
    (*pType)->kind = kind;
    return SK_SUCCESS;
}

void skDestroyPrimitiveType(SkPrimitiveType* pType) {
    assert(pType != NULL);

    free(*pType);
    *pType = NULL;
}

SkResult skInitDeferredType(char *pName, SkTypeProps props, SkDeferredType *pType) {
    if (!pType)
        return SK_FAILURE_BAD_HANDLE;

    *pType = malloc(sizeof(**pType));
    if (!*pType)
        return SK_FAILURE_OUT_OF_MEMORY;

    SkType base;
    base.tid = tid++;
    base.kind = SK_TYPE_KIND_DEFERRED;
    base.pName = pName;

    (*pType)->base = base;
    (*pType)->props = props;
    return SK_SUCCESS;
}

void skDestroyDeferredType(SkDeferredType *pType) {
    assert(pType != NULL);

    free(*pType);
    *pType = NULL;
}

SkResult skInitPointerType(SkType *pPointee, SkPointerType* pType) {
    if (!pType)
        return SK_FAILURE_BAD_HANDLE;

    *pType = malloc(sizeof(**pType));
    if (!*pType)
        return SK_FAILURE_OUT_OF_MEMORY;

    u32 size = strlen(pPointee->pName) + 2;
    char* name = calloc(sizeof(char), size);
    strcpy(name, pPointee->pName);
    strcat(name, "*");
    name[size - 1] = '\0';

    SkType base;
    base.tid = tid++;
    base.kind = SK_TYPE_KIND_POINTER;
    base.pName = name;
    
    (*pType)->base = base;
    (*pType)->pPointee = pPointee;
    return SK_SUCCESS;
}

void skDestroyPointerType(SkPointerType* pType) {
    assert(pType != NULL);

    free((*pType)->base.pName);
    (*pType)->base.pName = NULL;
    (*pType)->pPointee = NULL;

    free(*pType);
    *pType = NULL;
}
