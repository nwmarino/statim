#ifndef SKVOZ_INPUT_H_
#define SKVOZ_INPUT_H_

#include "common.h"

typedef struct {
    char*   pPath;
    char*   pContents;
}* SkInputFile;

SkResult skInitInputFile(char* pPath, char* pContents, SkInputFile* pFile);
SkResult skDestroyInputFile(SkInputFile* pFile);

typedef struct {
    SkInputFile     file;
    u32             line;
    u32             column;
} SkMetadata;

#endif // SKVOZ_INPUT_H_
