#ifndef STATIM_INTERNAL_AST_H_
#define STATIM_INTERNAL_AST_H_

#include "../include/ast.h"

#include "type.h"

struct StmRoot_T {
    struct StmTranslationUnit_T* pUnit;
    StmArray decls;
    struct StmPrimitiveType_T* pTypeVoid;
    struct StmPrimitiveType_T* pTypeBool;
    struct StmPrimitiveType_T* pTypeChar;
    struct StmPrimitiveType_T* pTypeI8;
    struct StmPrimitiveType_T* pTypeI16;
    struct StmPrimitiveType_T* pTypeI32;
    struct StmPrimitiveType_T* pTypeI64;
    struct StmPrimitiveType_T* pTypeU8;
    struct StmPrimitiveType_T* pTypeU16;
    struct StmPrimitiveType_T* pTypeU32;
    struct StmPrimitiveType_T* pTypeU64;
    struct StmPrimitiveType_T* pTypeF32;
    struct StmPrimitiveType_T* pTypeF64;
    StmArray functionTypes;
    StmArray structTypes;
};

struct StmDecl_T {
    StmDeclKind kind;
    StmMetadata meta;
    char*       pName;
};

struct StmStmt_T {
    StmStmtKind kind;
    StmMetadata meta;
};

struct StmExpr_T {
    StmExprKind         kind;
    StmMetadata         meta;
    StmExprProps        props;
    struct StmType_T*   pType;
};

struct StmFunctionDecl_T {
    struct StmDecl_T            base;
    struct StmFunctionType_T*   pType;
    StmArray                    params;
    struct StmScope_T*          pScope;
    struct StmStmt_T*           pStmt;
};

struct StmParamDecl_T {
    struct StmDecl_T    base;
    struct StmType_T*   pType;
};

struct StmBlockStmt_T {
    struct StmStmt_T    base;
    StmArray            stmts;
    struct StmScope_T*  pScope;
};

struct StmRetStmt_T {
    struct StmStmt_T    base;
    struct StmExpr_T*   pExpr;
};

struct StmIntegerExpr_T {
    struct StmExpr_T    base;
    i64                 value;
};

#endif // STATIM_INTERNAL_AST_H_
