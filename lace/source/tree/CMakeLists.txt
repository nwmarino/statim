set(TREE_SOURCES
    AST.cpp
    Defn.cpp
    Expr.cpp
    Printer.cpp
    Rune.cpp
    Scope.cpp
    SemanticAnalysis.cpp
    Stmt.cpp
    SymbolAnalysis.cpp
    Type.cpp
    TypeResolution.cpp
)

add_library(Tree
    STATIC
        ${TREE_SOURCES}
)

target_link_libraries(Tree
    PRIVATE
        Core
)
