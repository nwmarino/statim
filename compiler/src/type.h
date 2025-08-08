#ifndef STATIM_INTERNAL_TYPE_H_
#define STATIM_INTERNAL_TYPE_H_

#include "../include/type.h"

/// A base structure for all concrete types.
struct StmType_T {
    u32         tid;
    StmTypeKind kind;
};

/// A primitive type, built-in to the language.
struct StmPrimitiveType_T {
    struct StmType_T        base;
    StmPrimitiveTypeKind    kind;
};

/// A deferred type, to be resolved later.
struct StmDeferredType_T {
    struct StmType_T    base;
    StmTypeContext      context;
};

/// A pointer type, encapsulating a pointee.
struct StmPointerType_T {
    struct StmType_T    base;
    struct StmType_T*   pPointee;
};

#endif // STATIM_INTERNAL_TYPE_H_
