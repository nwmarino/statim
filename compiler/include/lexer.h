#ifndef STATIM_LEXER_H_
#define STATIM_LEXER_H_

#include "core.h"
#include "token.h"

#ifdef __cplusplus
    extern "C" {
#endif // __cplusplus

typedef struct StmLexer_T* StmLexer;

STM_API_ATTR StmResult STM_API_CALL stmInitLexer(StmInputFile* pFile, StmLexer* pLexer);
STM_API_ATTR StmResult STM_API_CALL stmDestroyLexer(StmLexer* pLexer);
STM_API_ATTR StmToken STM_API_CALL stmLexToken(StmLexer lexer);

#ifdef __cplusplus
    }
#endif // __cplusplus

#endif // STATIM_LEXER_H_
