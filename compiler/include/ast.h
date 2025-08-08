#ifndef STATIM_AST_H_
#define STATIM_AST_H_

#include "core.h"
#include "metadata.h"
#include "scope.h"
#include "type.h"

#ifdef __cplusplus
    extern "C" {
#endif // __cplusplus

typedef struct StmRoot_T* StmRoot;

typedef struct StmDecl_T* StmDecl;
typedef struct StmFunctionDecl_T* StmFunctionDecl;

typedef struct StmStmt_T* StmStmt;
typedef struct StmBlockStmt_T* StmBlockStmt;
typedef struct StmRetStmt_T* StmRetStmt;

typedef struct StmExpr_T* StmExpr;
typedef struct StmIntegerExpr_T* StmIntegerExpr;

/// Different kinds of AST declarations.
typedef enum StmDeclKind {
    STM_DECL_KIND_FUNCTION,
    STM_DECL_KIND_VARIABLE,
    STM_DECL_KIND_STRUCTURE,
    STM_DECL_KIND_ENUM,
} StmDeclKind;

/// Different kinds of AST statements.
typedef enum StmStmtKind {
    STM_STMT_KIND_BLOCK,
    STM_STMT_KIND_RET,
} StmStmtKind;

/// Different kinds of AST expressions.
typedef enum StmExprKind {
    STM_EXPR_KIND_INTEGER,
} StmExprKind;

/// Potential interpretations for expression values.
typedef enum StmExprProps {
    STM_EXPR_PROPERTIES_LVALUE,
    STM_EXPR_PROPERTIES_RVALUE,
    STM_EXPR_PROPERTIES_INITIALIZER,
} StmExprProps;

typedef struct StmFunctionDeclCreateInfo {
    StmMetadata     meta;
    char*           pName;
    StmArray        params;
    StmType         type;
    StmScope        scope;
    StmStmt         stmt;
} StmFunctionDeclCreateInfo;

typedef struct StmBlockStmtCreateInfo {
    StmMetadata     meta;
    StmArray        stmts;
    StmScope        scope;
} StmBlockStmtCreateInfo;

typedef struct StmRetStmtCreateInfo {
    StmMetadata     meta;
    StmExpr         expr;
} StmRetStmtCreateInfo;

typedef struct StmIntegerExprCreateInfo {
    StmMetadata     meta;
    StmExprProps    props;
    StmType         type;
    i64             value;
} StmIntegerExprCreateInfo;

STM_API_ATTR StmResult STM_API_CALL stmInitFunctionDecl(StmRoot root, StmFunctionDeclCreateInfo* pCreateInfo, StmFunctionDecl* pDecl);
STM_API_ATTR void STM_API_CALL stmDestroyFunctionDecl(StmFunctionDecl* pDecl);

#ifdef __cplusplus
    }
#endif // __cplusplus

#endif // STATIM_AST_H_
