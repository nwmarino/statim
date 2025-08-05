#include "metadata.h"
#include "common.h"

#include <assert.h>
#include <stdlib.h>

SkResult skInitInputFile(char* pPath, char* pContents, SkInputFile* pFile) {
    assert(pFile != NULL);

    *pFile = malloc(sizeof(**pFile));
    if (!*pFile)
        return SK_FAILURE_OUT_OF_MEMORY;

    (*pFile)->pPath = pPath;
    (*pFile)->pContents = pContents;
    return SK_SUCCESS;
}

SkResult skDestroyInputFile(SkInputFile* pFile) {
    assert(pFile != NULL);

    if (!*pFile)
        return SK_FAILURE_BAD_HANDLE;

    free(*pFile);
    *pFile = NULL;
    return SK_SUCCESS;
}
