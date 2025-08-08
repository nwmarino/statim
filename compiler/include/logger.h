#ifndef STATIM_LOGGER_H_
#define STATIM_LOGGER_H_

#include "core.h"
#include "metadata.h"

#ifdef __cplusplus
    extern "C" {
#endif // __cplusplus

/// Different levels of logger nessage severity.
typedef enum StmLoggerSeverity {
    STM_LOGGER_SEVERITY_INFO,
    STM_LOGGER_SEVERITY_WARN,
    STM_LOGGER_SEVERITY_ERROR,
    STM_LOGGER_SEVERITY_FATAL,
} StmLoggerSeverity;

/// Initialize the logger. If an output file is not provided, the default is stderr.
STM_API_ATTR void STM_API_CALL stmInitLogger(FILE* pOutput);

/// Log a message with the given severity.
STM_API_ATTR void STM_API_CALL stmLog(StmLoggerSeverity severity, char* pMessage, StmMetadata* pMeta);

/// Log a non-severe, informative message.
STM_API_ATTR void STM_API_CALL stmLogInfo(char* pMessage, StmMetadata* pMeta);

/// Log a non-severe warning message.
STM_API_ATTR void STM_API_CALL stmLogWarning(char* pMessage, StmMetadata* pMeta);

/// Log an error message.
STM_API_ATTR void STM_API_CALL stmLogError(char* pMessage, StmMetadata* pMeta);

/// Log a fatal message and end the current process.
STM_API_ATTR void STM_API_CALL stmLogFatal(char* pMessage, StmMetadata* pMeta);

#ifdef __cplusplus
    }
#endif // __cplusplus

#endif // STATIM_LOGGER_H_
