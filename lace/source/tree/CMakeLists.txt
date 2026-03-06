set(TREE_SOURCES
    Codegen.cpp
    Defn.cpp
    Expr.cpp
    NameResolution.cpp
    Printer.cpp
    Rib.cpp
    Rune.cpp
    Scope.cpp
    SemanticAnalysis.cpp
    Stmt.cpp
    SymbolAnalysis.cpp
    Type.cpp
    VisitorBase.cpp
)

add_library(Tree
    STATIC
        ${TREE_SOURCES}
)

target_link_libraries(Tree
    PRIVATE
        Core
        LIR
)
