set(TREE_SOURCES
    Defn.cpp
    Expr.cpp
    Printer.cpp
    Rib.cpp
    Rune.cpp
    Scope.cpp
    SemanticAnalysis.cpp
    Stmt.cpp
    SymbolAnalysis.cpp
    Type.cpp
    TypeResolution.cpp
    VisitorBase.cpp
)

add_library(Tree
    STATIC
        ${TREE_SOURCES}
)

target_link_libraries(Tree
    PRIVATE
        Core
)
