#include "metadata.h"
#include "common.h"

#include <assert.h>
#include <stdlib.h>

SkResult skInitInputFile(char* path, char* src, SkInputFile* pFile) {
    assert(pFile != SK_NULL);

    *pFile = malloc(sizeof(SkInputFile));
    if (!*pFile)
        return SK_FAILURE_OUT_OF_MEMORY;

    (*pFile)->path = path;
    (*pFile)->src = src;
    return SK_SUCCESS;
}

SkResult skDestroyInputFile(SkInputFile* pFile) {
    assert(pFile != SK_NULL);

    if (!*pFile)
        return SK_FAILURE_BAD_HANDLE;

    (*pFile)->path = NULL;
    (*pFile)->src = NULL;

    free(*pFile);
    *pFile = NULL;
    return SK_SUCCESS;
}
