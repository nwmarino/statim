#ifndef STATIM_CORE_INTERNAL_H_
#define STATIM_CORE_INTERNAL_H_

#include "../include/core.h"

#include "ast.h"

struct StmTranslationUnit_T {
    StmInputFile* pFile;
    struct StmRoot_T* pRoot;
};

#endif // STATIM_CORE_INTERNAL_H_
