#include "core.h"

struct StmArray_T {
    const void**    pData;
    u32             size;
    u32             capacity;
};

STM_API_ATTR StmResult STM_API_CALL stmInitTranslationUnit(StmInputFile* pFile, StmTranslationUnit* pUnit) {
    assert(pUnit != NULL && "(stmInitTranslationUnit) unit destination cannot be null.");

    *pUnit = malloc(sizeof(struct StmTranslationUnit_T));
    if (!*pUnit)
        return STM_FAILURE_OUT_OF_MEMORY;

    (*pUnit)->pFile = pFile;
    (*pUnit)->pRoot = NULL;
    return STM_SUCCESS;
}

STM_API_ATTR void STM_API_CALL stmDestroyTranslationUnit(StmTranslationUnit* pUnit) {
    assert(pUnit != NULL && "(stmDestroyTranslationUnit) unit cannot be null.");

    free(*pUnit);
    *pUnit = NULL;
}

STM_API_ATTR StmResult STM_API_CALL stmInitArray(u32 capacity, StmArray* pArray) {
    assert(pArray != NULL && "(stmInitArray) array destination cannot be null.");

    *pArray = malloc(sizeof(struct StmArray_T));
    if (!*pArray)
        return STM_FAILURE_OUT_OF_MEMORY;

    (*pArray)->pData = NULL;
    (*pArray)->size = 0;
    (*pArray)->capacity = capacity;

    if (capacity > 0)
        (*pArray)->pData = calloc(sizeof(void*), capacity);
    
    return STM_SUCCESS;
}

STM_API_ATTR void STM_API_CALL stmDestroyArray(StmArray* pArray) {
    assert(pArray != NULL && "(stmDestroyArray) array cannot be null.");

    if ((*pArray)->capacity > 0) {
        free((*pArray)->pData);
        (*pArray)->pData = NULL;
    }

    free(*pArray);
    *pArray = NULL;
}

STM_API_ATTR StmResult STM_API_CALL stmArrayPush(StmArray array, const void* pElement) {
    assert(array != NULL && "(stmArrayPush) array cannot be null.");

    if (array->size + 1 > array->capacity) {
        u32 new_capacity = array->capacity ? array->capacity * 2 : 1;
        const void** new_data = realloc(array->pData, new_capacity * sizeof(void*));
        if (!new_data)
            return STM_FAILURE_OUT_OF_MEMORY;

        array->pData = new_data;
        array->capacity = new_capacity;
    }

    array->pData[array->size++] = pElement;
    return STM_SUCCESS;
}

STM_API_ATTR const void* STM_API_CALL stmArrayPop(StmArray array) {
    assert(array != NULL && "(stmArrayPop) array cannot be null.");

    const void* tmp = array->pData[array->size];
    array->pData[--array->size] = NULL;
    return tmp;
}

STM_API_ATTR const void* STM_API_CALL stmArrayGet(StmArray array, u32 index) {
    assert(array != NULL && "(stmArrayGet) array cannot be null.");
    assert(index < array->size && "(stmArrayGet) array index out of bounds.");

    return array->pData[index];
}

STM_API_ATTR const void** STM_API_CALL stmArrayGetData(StmArray array) {
    assert(array != NULL && "(stmArrayData) array cannot be null.");

    return array->pData;
}

STM_API_ATTR u32 STM_API_CALL stmArrayGetSize(StmArray array) {
    assert(array != NULL && "(stmArraySize) array cannot be null.");

    return array->size;
}

STM_API_ATTR u32 STM_API_CALL stmArrayGetCapacity(StmArray array) {
    assert(array != NULL && "(stmArrayCapacity) array cannot be null.");

    return array->capacity;
}

STM_API_ATTR StmResult STM_API_CALL stmArrayReserve(StmArray array, u32 n) {
    assert(array != NULL && "(stmArrayReserve) array cannot be null.");

    u32 new_capacity = array->capacity + n;
    const void** new_data = realloc(array->pData, new_capacity * sizeof(void*));
    if (!new_data)
        return STM_FAILURE_OUT_OF_MEMORY;

    array->pData = new_data;
    array->capacity = new_capacity;
    return STM_SUCCESS;
}
