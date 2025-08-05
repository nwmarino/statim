#include "logger.h"

#include <assert.h>
#include <stdlib.h>

static FILE* logger_out = NULL; 

void logger_initialize(FILE* out) {
    if (!out)
        logger_out = stderr;
    else
        logger_out = out;
}

void logger_log(SkLoggerSeverity severity, char* msg, SkMetadata* pMeta) {
    switch (severity) {
    case SK_LOGGER_SEVERITY_INFO:
        return skLogInfo(msg, pMeta);
    case SK_LOGGER_SEVERITY_WARN:
        return skLogWarning(msg, pMeta);
    case SK_LOGGER_SEVERITY_ERROR:
        return skLogError(msg, pMeta);
    case SK_LOGGER_SEVERITY_FATAL:
        return skLogFatal(msg, pMeta);
    }
}

void skLogInfo(char* msg, SkMetadata* pMeta) {
    assert(logger_out != NULL);
    assert(msg != NULL);

    if (pMeta)
        fprintf(logger_out, "%s:%d:%d: ", pMeta->file->pPath, pMeta->line, pMeta->column);
    
    fprintf(logger_out, "info: %s\n", msg);
}

void skLogWarning(char* msg, SkMetadata* pMeta) {
    assert(logger_out != NULL);
    assert(msg != NULL);

    if (pMeta)
        fprintf(logger_out, "%s:%d:%d: ", pMeta->file->pPath, pMeta->line, pMeta->column);

    fprintf(logger_out, "warning: %s\n", msg);
}

void skLogError(char* msg, SkMetadata* pMeta) {
    assert(logger_out != NULL);
    assert(msg != NULL);

    if (pMeta)
        fprintf(logger_out, "%s:%d:%d: ", pMeta->file->pPath, pMeta->line, pMeta->column);

    fprintf(logger_out, "error: %s\n", msg);
}

void skLogFatal(char* msg, SkMetadata* pMeta) {
    assert(logger_out != NULL);
    assert(msg != NULL);
    
    if (pMeta)
        fprintf(logger_out, "%s:%d:%d: ", pMeta->file->pPath, pMeta->line, pMeta->column);

    fprintf(logger_out, "fatal: %s\n", msg);
    exit(EXIT_FAILURE);
}
