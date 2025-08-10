#ifndef STATIM_INTERNAL_PARSER_H_
#define STATIM_INTERNAL_PARSER_H_

#include "../include/ast.h"
#include "../include/parser.h"
#include "../include/scope.h"

struct StmParser_T {
    StmRoot     root;
    StmLexer    lexer;
    StmScope    scope;
    StmToken    token;
};

typedef enum ParseResult {
    PARSE_SUCCESS = 0x0,
    PARSE_FATAL,
} ParseResult;

ParseResult parse_type(StmParser parser, StmType* pType);

ParseResult parse_decl(StmParser parser, StmDecl* pDecl);
ParseResult parse_decl_function(StmParser parser, StmFunctionDecl* pDecl, StmToken nameTk);
ParseResult parse_decl_param(StmParser parser, StmParamDecl* pDecl);

ParseResult parse_stmt(StmParser parser, StmStmt* pStmt);
ParseResult parse_stmt_block(StmParser parser, StmBlockStmt* pStmt);
ParseResult parse_stmt_ret(StmParser parser, StmRetStmt* pStmt);

ParseResult parse_expr(StmParser parser, StmExpr* pExpr);
ParseResult parse_expr_integer(StmParser parser, StmIntegerExpr* pExpr);

#endif // STATIM_INTERNAL_PARSER_H_
