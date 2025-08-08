#include "type.h"

static u32 tid = 0;

static const char* get_primitive_name(StmPrimitiveTypeKind kind) {
    switch (kind) {
    case STM_PRIMITIVE_TYPE_KIND_BOOL: return "bool";
    case STM_PRIMITIVE_TYPE_KIND_CHAR: return "char";
    case STM_PRIMITIVE_TYPE_KIND_SINT8: return "i8";
    case STM_PRIMITIVE_TYPE_KIND_SINT16: return "i16";
    case STM_PRIMITIVE_TYPE_KIND_SINT32: return "i32";
    case STM_PRIMITIVE_TYPE_KIND_SINT64: return "i64";
    case STM_PRIMITIVE_TYPE_KIND_UINT8: return "u8";
    case STM_PRIMITIVE_TYPE_KIND_UINT16: return "u16";
    case STM_PRIMITIVE_TYPE_KIND_UINT32: return "u32";
    case STM_PRIMITIVE_TYPE_KIND_UINT64: return "u64";
    case STM_PRIMITIVE_TYPE_KIND_FLOAT32: return "f32";
    case STM_PRIMITIVE_TYPE_KIND_FLOAT64: return "f64";
    }
}

STM_API_ATTR StmResult STM_API_CALL stmInitPrimitiveType(StmPrimitiveTypeKind kind, StmPrimitiveType* pType) {
    assert(pType != NULL && "(stmInitPrimitiveType) type destination cannot be null.");

    *pType = malloc(sizeof(struct StmPrimitiveType_T));
    if (!*pType)
        return STM_FAILURE_OUT_OF_MEMORY;

    struct StmType_T base;
    base.tid = tid++;
    base.kind = STM_TYPE_KIND_PRIMITIVE;

    (*pType)->base = base;
    (*pType)->kind = kind;
    return STM_SUCCESS;
}

STM_API_ATTR void STM_API_CALL stmDestroyPrimitiveType(StmPrimitiveType* pType) {
    assert(pType != NULL && "(stmDestroyPrimitiveType) type cannot be null.");

    free(*pType);
    *pType = NULL;
}

STM_API_ATTR StmResult STM_API_CALL skInitDeferredType(StmTypeContext* pContext, StmDeferredType* pType) {
    assert(pContext != NULL && "(stmInitDeferredType) context cannot be null.");
    assert(pType != NULL && "(stmInitDeferredType) type destination cannot be null.");

    *pType = malloc(sizeof(struct StmDeferredType_T));
    if (!*pType)
        return STM_FAILURE_OUT_OF_MEMORY;

    struct StmType_T base;
    base.tid = tid++;
    base.kind = STM_TYPE_KIND_DEFERRED;

    (*pType)->base = base;
    (*pType)->context = *pContext;
    return STM_SUCCESS;
}

STM_API_ATTR void STM_API_CALL stmDestroyDeferredType(StmDeferredType* pType) {
    assert(pType != NULL && "(stmDestroyDeferredType) type cannot be null.");

    free(*pType);
    *pType = NULL;
}

STM_API_ATTR StmResult STM_API_CALL stmInitPointerType(StmType pointee, StmPointerType* pType) {
    assert(pointee != NULL && "(stmInitPointerType) pointee cannot be null.");
    assert(pType != NULL && "(stmInitPointerType) type destination cannot be null.");

    *pType = malloc(sizeof(struct StmPointerType_T));
    if (!*pType)
        return STM_FAILURE_OUT_OF_MEMORY;

    struct StmType_T base;
    base.tid = tid++;
    base.kind = STM_TYPE_KIND_POINTER;
    
    (*pType)->base = base;
    (*pType)->pPointee = pointee;
    return STM_SUCCESS;
}

STM_API_ATTR void STM_API_CALL stmDestroyPointerType(StmPointerType* pType) {
    assert(pType != NULL && "(stmDestroyPointerType) type cannot be null.");

    (*pType)->pPointee = NULL;

    free(*pType);
    *pType = NULL;
}
