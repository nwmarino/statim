#ifndef STATIM_INTERNAL_SCOPE_H_
#define STATIM_INTERNAL_SCOPE_H_

#include "../include/scope.h"

/// A node in a scope tree, linked to scoped AST nodes.
struct StmScope_T {
    struct StmScope_T*   pParent;
    StmScopeProps        props;
    StmArray             decls;
};

#endif // STATIM_INTERNAL_SCOPE_H_
