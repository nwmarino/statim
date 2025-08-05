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

void logger_log(SkLoggerSeverity severity, char* msg) {
    switch (severity) {
    case SK_LOGGER_SEVERITY_INFO:
        return skLogInfo(msg);
    case SK_LOGGER_SEVERITY_WARN:
        return skLogWarning(msg);
    case SK_LOGGER_SEVERITY_ERROR:
        return skLogError(msg);
    case SK_LOGGER_SEVERITY_FATAL:
        return skLogFatal(msg);
    }
}

void skLogInfo(char* msg) {
    assert(logger_out != NULL);
    assert(msg != NULL);

    fprintf(logger_out, "[INFO] %s\n", msg);
}

void skLogWarning(char* msg) {
    assert(logger_out != NULL);
    assert(msg != NULL);

    fprintf(logger_out, "[WARN] %s\n", msg);
}

void skLogError(char* msg) {
    assert(logger_out != NULL);
    assert(msg != NULL);

    fprintf(logger_out, "[ERROR] %s\n", msg);
}

void skLogFatal(char* msg) {
    assert(logger_out != NULL);
    assert(msg != NULL);
    
    fprintf(logger_out, "[FATAL] %s\n", msg);
    exit(EXIT_FAILURE);
}
