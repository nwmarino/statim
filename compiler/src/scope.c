#include "../include/ast.h"
#include "../include/scope.h"

#include "ast.h"

/// A node in a scope tree, linked to scoped AST nodes.
struct StmScope_T {
    struct StmScope_T*   pParent;
    StmScopeProps        props;
    StmArray             decls;
};

STM_API_ATTR StmResult STM_API_CALL skInitScope(StmScope parent, StmScopeProps props, StmScope* pScope) {
    assert(pScope && "(stmInitScope) scope destination cannot be null.");

    *pScope = malloc(sizeof(struct StmScope_T));
    if (!*pScope)
        return STM_FAILURE_OUT_OF_MEMORY;

    (*pScope)->pParent = parent;
    (*pScope)->props = props;

    return stmInitArray(0, &(*pScope)->decls);
}

STM_API_ATTR void STM_API_CALL stmDestroyScope(StmScope* pScope) {
    assert(pScope != NULL && "(skDestroyScope) scope cannot be null.");

    stmDestroyArray(&(*pScope)->decls);

    free(*pScope);
    *pScope = NULL;
}

STM_API_ATTR StmResult STM_API_CALL skScopeAddDecl(StmScope pScope, StmDecl decl) {
    assert(pScope != NULL && "(skScopeAddDecl) scope cannot be null.");
    assert(decl != NULL && "(skScopeAddDecl) declaration cannot be null.");

    return stmArrayPush(pScope->decls, decl);
}

STM_API_ATTR StmDecl STM_API_CALL skScopeGetDecl(StmScope pScope, char* pName) {
    assert(pScope != NULL && "(skScopeGetDecl) scope cannot be null.");
    
    for (u32 idx = 0, e = stmArrayGetSize(pScope->decls); idx != e; ++idx) {
        StmDecl decl = (StmDecl) stmArrayGet(pScope->decls, idx);
        if (!strcmp(pName, decl->pName))
            return decl;
    }

    if (pScope->pParent)
        return skScopeGetDecl(pScope->pParent, pName);

    return NULL;
}
