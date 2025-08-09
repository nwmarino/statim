#include "../include/type.h"

#include "ast.h"
#include "core.h"

STM_API_ATTR StmResult STM_API_CALL stmInitRoot(StmTranslationUnit unit, StmRoot* pRoot) {
    assert(unit != NULL && "(stmInitRoot) translation unit cannot be null.");
    assert(pRoot != NULL && "(stmInitRoot) root destination cannot be null.");

    *pRoot = malloc(sizeof(struct StmRoot_T));
    if (!*pRoot)
        return STM_FAILURE_OUT_OF_MEMORY;

    unit->pRoot = *pRoot;

    (*pRoot)->pUnit = unit;
    
    stmInitArray(0, &(*pRoot)->decls);
    stmInitArray(0, &(*pRoot)->functionTypes);

    stmInitPrimitiveType(STM_PRIMITIVE_TYPE_KIND_VOID, &(*pRoot)->pTypeVoid);
    stmInitPrimitiveType(STM_PRIMITIVE_TYPE_KIND_BOOL, &(*pRoot)->pTypeBool);
    stmInitPrimitiveType(STM_PRIMITIVE_TYPE_KIND_CHAR, &(*pRoot)->pTypeChar);
    stmInitPrimitiveType(STM_PRIMITIVE_TYPE_KIND_SINT8, &(*pRoot)->pTypeI8);
    stmInitPrimitiveType(STM_PRIMITIVE_TYPE_KIND_SINT16, &(*pRoot)->pTypeI16);
    stmInitPrimitiveType(STM_PRIMITIVE_TYPE_KIND_SINT32, &(*pRoot)->pTypeI32);
    stmInitPrimitiveType(STM_PRIMITIVE_TYPE_KIND_SINT64, &(*pRoot)->pTypeI64);
    stmInitPrimitiveType(STM_PRIMITIVE_TYPE_KIND_UINT8, &(*pRoot)->pTypeU8);
    stmInitPrimitiveType(STM_PRIMITIVE_TYPE_KIND_UINT16, &(*pRoot)->pTypeU16);
    stmInitPrimitiveType(STM_PRIMITIVE_TYPE_KIND_UINT32, &(*pRoot)->pTypeU32);
    stmInitPrimitiveType(STM_PRIMITIVE_TYPE_KIND_UINT64, &(*pRoot)->pTypeU64);
    return STM_SUCCESS;
}

STM_API_ATTR void STM_API_CALL stmDestroyRoot(StmRoot* pRoot) {
    assert(pRoot != NULL && "(stmDestroyRoot) root cannot be null.");

    for (u32 idx = 0, e = stmArrayGetSize((*pRoot)->decls); idx != e; ++idx) {
        stmDestroyDecl((StmDecl*) stmArrayGet((*pRoot)->decls, idx));
    }

    for (u32 idx = 0, e = stmArrayGetSize((*pRoot)->functionTypes); idx != e; ++idx) {

    }

    stmDestroyPrimitiveType(&(*pRoot)->pTypeVoid);
    stmDestroyPrimitiveType(&(*pRoot)->pTypeBool);
    stmDestroyPrimitiveType(&(*pRoot)->pTypeChar);
    stmDestroyPrimitiveType(&(*pRoot)->pTypeI8);
    stmDestroyPrimitiveType(&(*pRoot)->pTypeI16);
    stmDestroyPrimitiveType(&(*pRoot)->pTypeI32);
    stmDestroyPrimitiveType(&(*pRoot)->pTypeI64);
    stmDestroyPrimitiveType(&(*pRoot)->pTypeU8);
    stmDestroyPrimitiveType(&(*pRoot)->pTypeU16);
    stmDestroyPrimitiveType(&(*pRoot)->pTypeU32);
    stmDestroyPrimitiveType(&(*pRoot)->pTypeU64);
    stmDestroyPrimitiveType(&(*pRoot)->pTypeF32);
    stmDestroyPrimitiveType(&(*pRoot)->pTypeF64);

    free(*pRoot);
    *pRoot = NULL;
}

STM_API_ATTR StmPrimitiveType STM_API_CALL stmGetBoolType(StmRoot root) {
    assert(root != NULL && "(stmGetBoolType) root cannot be null.");
    return root->pTypeBool;
}

STM_API_ATTR StmPrimitiveType STM_API_CALL stmGetCharType(StmRoot root) {
    assert(root != NULL && "(stmGetCharType) root cannot be null.");
    return root->pTypeChar;
}

STM_API_ATTR StmPrimitiveType STM_API_CALL stmGetSint8Type(StmRoot root) {
    assert(root != NULL && "(stmGetSint8Type) root cannot be null.");
    return root->pTypeI8;
}

STM_API_ATTR StmPrimitiveType STM_API_CALL stmGetSint16ype(StmRoot root) {
    assert(root != NULL && "(stmGetSint16ype) root cannot be null.");
    return root->pTypeI16;
}

STM_API_ATTR StmPrimitiveType STM_API_CALL stmGetSint32Type(StmRoot root) {
    assert(root != NULL && "(stmGetSint32Type) root cannot be null.");
    return root->pTypeI32;
}

STM_API_ATTR StmPrimitiveType STM_API_CALL stmGetSint64Type(StmRoot root) {
    assert(root != NULL && "(stmGetSint64Type) root cannot be null.");
    return root->pTypeI64;
}

STM_API_ATTR StmPrimitiveType STM_API_CALL stmGetUint8Type(StmRoot root) {
    assert(root != NULL && "(stmGetUint8Type) root cannot be null.");
    return root->pTypeU8;
}

STM_API_ATTR StmPrimitiveType STM_API_CALL stmGetUint16ype(StmRoot root) {
    assert(root != NULL && "(stmGetUint16ype) root cannot be null.");
    return root->pTypeU16;
}

STM_API_ATTR StmPrimitiveType STM_API_CALL stmGetUint32Type(StmRoot root) {
    assert(root != NULL && "(stmGetUint32Type) root cannot be null.");
    return root->pTypeU32;
}

STM_API_ATTR StmPrimitiveType STM_API_CALL stmGetUint64Type(StmRoot root) {
    assert(root != NULL && "(stmGetUint64Type) root cannot be null.");
    return root->pTypeU64;
}

STM_API_ATTR StmPrimitiveType STM_API_CALL stmGetFloat32Type(StmRoot root) {
    assert(root != NULL && "(stmGetFloat32Type) root cannot be null.");
    return root->pTypeF32;
}   

STM_API_ATTR StmPrimitiveType STM_API_CALL stmGetFloat64Type(StmRoot root) {
    assert(root != NULL && "(stmGetFloat64Type) root cannot be null.");
    return root->pTypeF64;
}

STM_API_ATTR void STM_API_CALL stmDestroyDecl(StmDecl* pDecl) {
    assert(pDecl != NULL && "(stmDestroyDecl) decl cannot be null.");

    switch ((*pDecl)->kind) {
    case STM_DECL_KIND_FUNCTION:
        return stmDestroyFunctionDecl((StmFunctionDecl*) pDecl);
    case STM_DECL_KIND_PARAMETER:
        return stmDestroyParamDecl((StmParamDecl*) pDecl);
    }

    assert(STM_FALSE && "(stmDestoryDecl) unknown declaration kind.");
}

STM_API_ATTR StmResult STM_API_CALL stmInitFunctionDecl(StmRoot root, StmFunctionDeclCreateInfo* pCreateInfo, StmFunctionDecl* pDecl) {
    assert(root != NULL && "(stmInitFunctionDecl) root cannot be null.");
    assert(pCreateInfo != NULL && "(stmInitFunctionDecl) creation info cannot be null.");
    assert(pDecl != NULL && "(stmInitFunctionDecl) decl destination cannot be null.");

    *pDecl = malloc(sizeof(struct StmFunctionDecl_T));
    if (!*pDecl)
        return STM_FAILURE_OUT_OF_MEMORY;

    struct StmDecl_T base;
    base.kind = STM_DECL_KIND_FUNCTION;
    base.meta = pCreateInfo->meta;
    base.pName = pCreateInfo->pName;

    (*pDecl)->base = base;
    (*pDecl)->pType = pCreateInfo->type;
    (*pDecl)->params = pCreateInfo->params;
    (*pDecl)->pScope = pCreateInfo->scope;
    (*pDecl)->pStmt = pCreateInfo->stmt;
    return STM_SUCCESS;
}

STM_API_ATTR void STM_API_CALL stmDestroyFunctionDecl(StmFunctionDecl* pDecl) {
    assert(pDecl != NULL && "(stmDestroyFunctionDecl) decl cannot be null.");

    free((*pDecl)->base.pName);

    stmDestroyFunctionType(&(*pDecl)->pType);
    
    for (u32 idx = 0, e = stmArrayGetSize((*pDecl)->params); idx != e; ++idx) {
        // destroy param
        stmDestroyArray(&(*pDecl)->params);
    }

    stmDestroyScope(&(*pDecl)->pScope);
    stmDestroyStmt(&(*pDecl)->pStmt);

    free(*pDecl);
    *pDecl = NULL;
}

STM_API_ATTR StmResult STM_API_CALL stmInitParamDecl(StmRoot root, StmParamDeclCreateInfo* pCreateInfo, StmParamDecl* pDecl) {
    assert(root != NULL && "(stmInitParamDecl) root cannot be null.");
    assert(pCreateInfo != NULL && "(stmInitParamDecl) creation info cannot be null.");
    assert(pDecl != NULL && "(stmInitParamDecl) decl destination cannot be null.");

    *pDecl = malloc(sizeof(struct StmParamDecl_T));
    if (!*pDecl)
        return STM_FAILURE_OUT_OF_MEMORY;

    struct StmDecl_T base;
    base.kind = STM_DECL_KIND_PARAMETER;
    base.meta = pCreateInfo->meta;
    base.pName = pCreateInfo->pName;

    (*pDecl)->base = base;
    (*pDecl)->pType = pCreateInfo->type;
    return STM_SUCCESS;
}

STM_API_ATTR void STM_API_CALL stmDestroyParamDecl(StmParamDecl* pDecl) {
    assert(pDecl != NULL);

    free((*pDecl)->base.pName);
    free(*pDecl);
    *pDecl = NULL;
}

STM_API_ATTR void STM_API_CALL stmDestroyStmt(StmStmt* pStmt) {
    assert(pStmt != NULL && "(stmDestroyStmt) stmt cannot be null.");

    switch ((*pStmt)->kind) {
    case STM_STMT_KIND_BLOCK:
        return stmDestroyBlockStmt((StmBlockStmt*) pStmt);
    case STM_STMT_KIND_RET:
        return stmDestroyRetStmt((StmRetStmt*) pStmt);
    }

    assert(STM_FALSE && "(stmDestroyStmt) unknown statement kind.");
}

STM_API_ATTR StmResult STM_API_CALL stmInitBlockStmt(StmRoot root, StmBlockStmtCreateInfo* pCreateInfo, StmBlockStmt* pStmt) {
    assert(root);
    assert(pCreateInfo);
    assert(pStmt);

    *pStmt = malloc(sizeof(struct StmBlockStmt_T));
    if (!*pStmt)
        return STM_FAILURE_OUT_OF_MEMORY;

    struct StmStmt_T base;
    base.kind = STM_STMT_KIND_BLOCK;
    base.meta = pCreateInfo->meta;

    (*pStmt)->base = base;
    (*pStmt)->pScope = pCreateInfo->scope;
    (*pStmt)->stmts = pCreateInfo->stmts;
    return STM_SUCCESS;
}

STM_API_ATTR void STM_API_CALL stmDestroyBlockStmt(StmBlockStmt* pStmt) {
    assert(pStmt);

    stmDestroyScope(&(*pStmt)->pScope);
    (*pStmt)->pScope = NULL;

    for (u32 idx = 0, e = stmArrayGetSize((*pStmt)->stmts); idx != e; ++idx) {
        stmDestroyStmt((StmStmt*) stmArrayGet((*pStmt)->stmts, idx));
    }

    free(*pStmt);
    *pStmt = NULL;
}

STM_API_ATTR StmResult STM_API_CALL stmInitRetStmt(StmRoot root, StmRetStmtCreateInfo* pCreateInfo, StmRetStmt* pStmt) {
    assert(root);
    assert(pCreateInfo);
    assert(pStmt);

    *pStmt = malloc(sizeof(struct StmRetStmt_T));
    if (!*pStmt)
        return STM_FAILURE_OUT_OF_MEMORY;

    struct StmStmt_T base;
    base.kind = STM_STMT_KIND_RET;
    base.meta = pCreateInfo->meta;

    (*pStmt)->base = base;
    (*pStmt)->pExpr = pCreateInfo->expr;
    return STM_SUCCESS;
}

STM_API_ATTR void STM_API_CALL stmDestroyRetStmt(StmRetStmt* pStmt) {
    assert(pStmt);

    stmDestroyExpr(&(*pStmt)->pExpr);
    (*pStmt)->pExpr = NULL;

    free(*pStmt);
    *pStmt = NULL;
}

STM_API_ATTR void STM_API_CALL stmDestroyExpr(StmExpr* pExpr) {
    assert(pExpr != NULL && "(stmDestroyExpr) expr cannot be null.");

    switch ((*pExpr)->kind) {
    case STM_EXPR_KIND_INTEGER:
        return stmDestroyIntegerExpr((StmIntegerExpr*) pExpr);
    }

    assert(STM_FALSE && "(stmDestroyExpr) unknown expression kind.");
}

STM_API_ATTR StmResult STM_API_CALL stmInitIntegerExpr(StmRoot root, StmIntegerExprCreateInfo* pCreateInfo, StmIntegerExpr* pExpr) {
    assert(root != NULL);
    assert(pCreateInfo != NULL);
    assert(pExpr != NULL);

    *pExpr = malloc(sizeof(struct StmIntegerExpr_T));
    if (!*pExpr)
        return STM_FAILURE_OUT_OF_MEMORY;

    struct StmExpr_T base;
    base.kind = STM_EXPR_KIND_INTEGER;
    base.meta = pCreateInfo->meta;
    base.props = pCreateInfo->props;
    base.pType = pCreateInfo->type;

    (*pExpr)->base = base;
    (*pExpr)->value = pCreateInfo->value;
    return STM_SUCCESS;
}

STM_API_ATTR void STM_API_CALL stmDestroyIntegerExpr(StmIntegerExpr* pExpr) {
    assert(pExpr);

    free(*pExpr);
    *pExpr = NULL;
}
