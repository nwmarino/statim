#ifndef STATIM_SCOPE_H_
#define STATIM_SCOPE_H_

#include "core.h"

#ifdef __cplusplus
    extern "C" {
#endif // __cplusplus

typedef struct StmDecl_T* StmDecl;
typedef struct StmScope_T* StmScope;

/// Locational properties for a scope.
typedef enum StmScopeProps : i8 {
    STM_SCOPE_PROPERTIES_GLOBAL_SCOPE,
    STM_SCOPE_PROPERTIES_FUNCTION_SCOPE,
    STM_SCOPE_PROPERTIES_BLOCK_SCOPE,
    STM_SCOPE_PROPERTIES_STRUCTURE_SCOPE,
} StmScopeProps;

STM_API_ATTR StmResult STM_API_CALL stmInitScope(StmScope parent, StmScopeProps props, StmScope* pScope);
STM_API_ATTR void STM_API_CALL stmDestroyScope(StmScope* pScope);
STM_API_ATTR StmResult STM_API_CALL stmScopeAddDecl(StmScope scope, StmDecl decl);
STM_API_ATTR StmDecl STM_API_CALL stmScopeGetDecl(StmScope scope, char* pName);

#ifdef __cplusplus
    }
#endif // __cplusplus

#endif // STATIM_SCOPE_H_
