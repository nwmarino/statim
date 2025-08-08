#ifndef STATIM_INTERNAL_AST_H_
#define STATIM_INTERNAL_AST_H_

#include "../include/ast.h"

struct StmRoot_T {
    StmArray decls;
};

struct StmDecl_T {
    StmMetadata meta;
    char*       pName;
};

struct StmStmt_T {
    StmMetadata meta;
};

struct StmExpr_T {
    StmMetadata         meta;
    StmExprProps        props;
    struct StmType_T*   pType;
};

struct SkFunctionDecl_T {
    struct StmDecl_T           base;
    struct StmType_T*   pType;
    StmArray            params;
    struct StmScope_T*  pScope;
    struct StmStmt_T*   pStmt;
};

struct SkBlockStmt_T {
    struct StmStmt_T    base;
    StmArray            stmts;
    struct StmScope_T*  pScope;
};

struct SkRetStmt_T {
    struct StmStmt_T    base;
    struct StmExpr_T*   pExpr;
};

struct SkIntegerExpr_T {
    struct StmExpr_T    base;
    i64                 value;
};

#endif // STATIM_INTERNAL_AST_H_
