#ifndef SKVOZ_INPUT_H_
#define SKVOZ_INPUT_H_

#include "common.h"

typedef struct {
    SkString    path;
    SkString    contents;
}* SkInputFile;

SkResult skInitInputFile(SkString path, SkString src, SkInputFile* pFile);
SkResult skDestroyInputFile(SkInputFile* pFile);

typedef struct {
    SkInputFile     pFile;
    u32             line;
    u32             column;
} SkMetadata;

#endif // SKVOZ_INPUT_H_
