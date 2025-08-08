#include "../include/logger.h"

static FILE* logger_out = NULL; 

void stmInitLogger(FILE* pOutput) {
    if (!pOutput)
        logger_out = stderr;
    else
        logger_out = pOutput;
}

void stmLog(StmLoggerSeverity severity, char* pMessage, StmMetadata* pMeta) {
    switch (severity) {
    case STM_LOGGER_SEVERITY_INFO:
        return stmLogInfo(pMessage, pMeta);
    case STM_LOGGER_SEVERITY_WARN:
        return stmLogWarning(pMessage, pMeta);
    case STM_LOGGER_SEVERITY_ERROR:
        return stmLogError(pMessage, pMeta);
    case STM_LOGGER_SEVERITY_FATAL:
        return stmLogFatal(pMessage, pMeta);
    }
}

void stmLogInfo(char* pMessage, StmMetadata* pMeta) {
    assert(logger_out != NULL && "(stmLogInfo) logger has not been initialized.");
    assert(pMessage != NULL && "(stmLogInfo) message cannot be null.");

    if (pMeta)
        fprintf(logger_out, "%s:%d:%d: ", pMeta->pFile->pPath, pMeta->line, pMeta->column);
    
    fprintf(logger_out, "info: %s\n", pMessage);
}

void stmLogWarning(char* pMessage, StmMetadata* pMeta) {
    assert(logger_out != NULL && "(stmLogWarning) logger has not been initialized.");
    assert(pMessage != NULL && "(stmLogWarning) message cannot be null.");

    if (pMeta)
        fprintf(logger_out, "%s:%d:%d: ", pMeta->pFile->pPath, pMeta->line, pMeta->column);

    fprintf(logger_out, "warning: %s\n", pMessage);
}

void stmLogError(char* pMessage, StmMetadata* pMeta) {
    assert(logger_out != NULL && "(stmLogError) logger has not been initialized.");
    assert(pMessage != NULL && "(stmLogError) message cannot be null.");

    if (pMeta)
        fprintf(logger_out, "%s:%d:%d: ", pMeta->pFile->pPath, pMeta->line, pMeta->column);

    fprintf(logger_out, "error: %s\n", pMessage);
}

void stmLogFatal(char* pMessage, StmMetadata* pMeta) {
    assert(logger_out != NULL && "(stmLogFatal) logger has not been initialized.");
    assert(pMessage != NULL && "(stmLogFatal) message cannot be null.");
    
    if (pMeta)
        fprintf(logger_out, "%s:%d:%d: ", pMeta->pFile->pPath, pMeta->line, pMeta->column);

    fprintf(logger_out, "fatal: %s\n", pMessage);
    exit(EXIT_FAILURE);
}
