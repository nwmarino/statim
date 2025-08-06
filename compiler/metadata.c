#include "metadata.h"
#include "common.h"

#include <assert.h>
#include <stdlib.h>

SkResult skInitInputFile(char* pPath, char* pContents, SkInputFile* pFile) {
    if (!pFile)
        return SK_FAILURE_BAD_HANDLE;

    *pFile = malloc(sizeof(**pFile));
    if (!*pFile)
        return SK_FAILURE_OUT_OF_MEMORY;

    (*pFile)->pPath = pPath;
    (*pFile)->pContents = pContents;
    return SK_SUCCESS;
}

void skDestroyInputFile(SkInputFile* pFile) {
    assert(pFile != NULL);

    free(*pFile);
    *pFile = NULL;
}
