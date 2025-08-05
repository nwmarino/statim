#ifndef SKVOZ_LOGGER_H_
#define SKVOZ_LOGGER_H_

#include "common.h"

#include <stdio.h>

typedef enum {
    SK_LOGGER_SEVERITY_INFO,
    SK_LOGGER_SEVERITY_WARN,
    SK_LOGGER_SEVERITY_ERROR,
    SK_LOGGER_SEVERITY_FATAL,
} SkLoggerSeverity;

/// Initialize the logger. If an output file is not provided, the default is stderr.
void skInitLogger(FILE* out);

/// Log a message with the given severity.
void skLog(SkLoggerSeverity severity, char* msg);

/// Log an non-severe, informative message.
void skLogInfo(char* msg);

/// Log an non-severe warning message.
void skLogWarning(char* msg);

/// Log an error message.
void skLogError(char* msg);

/// Log a fatal message and end the current process.
void skLogFatal(char* msg);

#endif // SKVOZ_LOGGER_H_
