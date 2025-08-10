#include "../include/logger.h"

#include "ast.h"
#include "parser.h"
#include "scope.h"

static void next(StmParser parser) {
    parser->token = stmLexToken(parser->lexer);
}

static StmBool8 match(StmParser parser, StmTokenKind kind) {
    assert(parser != NULL);
    return parser->token.kind == kind;
}

static StmBool8 match_kw(StmParser parser, const char* kw) {
    assert(parser != NULL);
    assert(kw != NULL);
    return parser->token.kind == STM_TOKEN_KIND_IDENTIFIER && !strcmp(parser->token.pValue, kw);
}

static void expect(StmParser parser, StmTokenKind kind) {
    assert(parser != NULL);
    if (parser->token.kind == kind)
        return;

    char buf[128];
    snprintf(buf, 128, "expected token: %s", stmStringifyToken(kind));
    stmLogFatal(buf, &parser->token.meta);
    assert(STM_FALSE);
}

static void expect_kw(StmParser parser, const char* kw) {
    assert(parser != NULL);
    assert(kw != NULL);

    if (match(parser, STM_TOKEN_KIND_IDENTIFIER) 
      && !strcmp(parser->token.pValue, kw))
        return;

    char buf[128];
    snprintf(buf, 128, "expected keyword: %s", kw);
    stmLogFatal(buf, &parser->token.meta);
    assert(STM_FALSE);
}

static StmScope enter_scope(StmParser parser, StmScopeProps props) {
    stmInitScope(parser->scope, props, &parser->scope);
    return parser->scope;
}

static void exit_scope(StmParser parser) {
    parser->scope = parser->scope->pParent;
}

ParseResult parse_type(StmParser parser, StmType* pType) {
    
}

ParseResult parse_decl(StmParser parser, StmDecl* pDecl) {
    expect(parser, STM_TOKEN_KIND_IDENTIFIER);
    
    StmToken nameTk = parser->token;
    next(parser); // name

    expect(parser, STM_TOKEN_KIND_PATH);
    next(parser); // '::'

    if (match(parser, STM_TOKEN_KIND_SET_PAREN))
        return parse_decl_function(parser, (StmFunctionDecl*) pDecl, nameTk);

    return PARSE_FATAL;
}

ParseResult parse_decl_function(StmParser parser, StmFunctionDecl* pDecl, StmToken nameTk) {    
    next(parser); // '('

    StmFunctionDeclCreateInfo info;
    info.meta = nameTk.meta;
    info.pName = nameTk.pValue;
    info.scope = enter_scope(parser, STM_SCOPE_PROPERTIES_FUNCTION_SCOPE);

    if (stmInitArray(0, &info.params) != STM_SUCCESS)
        return PARSE_FATAL;

    while (match(parser, STM_TOKEN_KIND_END_PAREN) != STM_TRUE) {

    }

    next(parser); // ')'

    StmType retType;
    parse_type(parser, &retType);
    StmArray paramTypes;
    stmInitArray(stmArrayGetSize(info.params), &paramTypes);
    for (u32 idx = 0, e = stmArrayGetSize(info.params); idx != e; ++idx)
        stmArrayPush(paramTypes, ((StmParamDecl) stmArrayGet(info.params, idx))->pType);

    stmInitFunctionType(retType, paramTypes, &info.type);

    if (match(parser, STM_TOKEN_KIND_SET_BRACE)) {
        if (parse_stmt(parser, &info.stmt) != PARSE_SUCCESS)
            stmLogFatal("expected function body", &parser->token.meta);
    } else {
        expect(parser, STM_TOKEN_KIND_SEMICOLON);
        next(parser); // ';'
    }

    exit_scope(parser);

    if (stmInitFunctionDecl(parser->root, &info, pDecl) != STM_SUCCESS)
        return PARSE_FATAL;

    stmScopeAddDecl(parser->scope, (StmDecl) *pDecl);
    return PARSE_SUCCESS;
}

ParseResult parse_decl_param(StmParser parser, StmParamDecl* pDecl) {
    return PARSE_SUCCESS;
}


ParseResult parse_stmt(StmParser parser, StmStmt* pStmt) {
    if (match(parser, STM_TOKEN_KIND_SET_BRACE))
        return parse_stmt_block(parser, (StmBlockStmt*) pStmt);
    
    if (match_kw(parser, "ret"))
        return parse_stmt_ret(parser, (StmRetStmt*) pStmt);

    return PARSE_FATAL;
}

ParseResult parse_stmt_block(StmParser parser, StmBlockStmt* pStmt) {
    StmBlockStmtCreateInfo info;
    info.meta = parser->token.meta;
    info.scope = enter_scope(parser, STM_SCOPE_PROPERTIES_BLOCK_SCOPE);

    if (stmInitArray(1, &info.stmts) != STM_SUCCESS)
        return PARSE_FATAL;

    next(parser); // '{'
    while (match(parser, STM_TOKEN_KIND_END_BRACE) != STM_TRUE) {
        StmStmt stmt;
        if (parse_stmt(parser, &stmt) != PARSE_SUCCESS)
            return PARSE_FATAL;

        while (match(parser, STM_TOKEN_KIND_SEMICOLON) == STM_TRUE)
            next(parser); // ';'

        stmArrayPush(info.stmts, stmt);
    }

    next(parser); // '}'
    exit_scope(parser);

    if (stmInitBlockStmt(parser->root, &info, pStmt) != STM_SUCCESS)
        return PARSE_FATAL;

    return PARSE_SUCCESS;
}

ParseResult parse_stmt_ret(StmParser parser, StmRetStmt* pStmt) {
    StmRetStmtCreateInfo info;
    info.meta = parser->token.meta;

    free(parser->token.pValue);
    next(parser); // 'ret'

    if (parse_expr(parser, &info.expr) != PARSE_SUCCESS)
        return PARSE_FATAL;

    if (stmInitRetStmt(parser->root, &info, pStmt) != STM_SUCCESS)
        return PARSE_FATAL;

    return PARSE_SUCCESS;
}


ParseResult parse_expr(StmParser parser, StmExpr* pExpr) {
    return parse_expr_integer(parser, (StmIntegerExpr*) pExpr);
}

ParseResult parse_expr_integer(StmParser parser, StmIntegerExpr* pExpr) {
    StmIntegerExprCreateInfo info;
    info.meta = parser->token.meta;
    info.props = STM_EXPR_PROPERTIES_RVALUE;
    info.type = (StmType) stmGetSint64Type(parser->root);
    info.value = atol(parser->token.pValue);

    free(parser->token.pValue);

    if (stmInitIntegerExpr(parser->root, &info, pExpr) != STM_SUCCESS)
        return PARSE_FATAL;

    return PARSE_SUCCESS;
}
