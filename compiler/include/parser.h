#ifndef STATIM_PARSER_H_
#define STATIM_PARSER_H_

#include "core.h"
#include "lexer.h"

typedef struct StmParser_T* StmParser;

STM_API_ATTR StmResult STM_API_CALL stmInitParser(StmParser* pParser);
STM_API_ATTR void STM_API_CALL stmDestroyParser(StmParser* pParser);
STM_API_ATTR void STM_API_CALL stmParseTree(StmParser parser, StmTranslationUnit unit);

#endif // STATIM_PARSER_H_
