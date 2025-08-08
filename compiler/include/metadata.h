#ifndef STATIM_METADATA_H_
#define STATIM_METADATA_H_

#include "core.h"

/// Representation of an input file to the compiler.
typedef struct StmInputFile {
    char*   pPath;
    char*   pContents;
} StmInputFile;

/// File-based source metadata attached to IR nodes.
typedef struct StmMetadata {
    StmInputFile*    pFile;
    u32             line;
    u32             column;
} StmMetadata;

#endif // STATIM_METADATA_H_
