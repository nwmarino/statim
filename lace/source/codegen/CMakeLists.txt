set(CODEGEN_SOURCES
    LIRCodegen.cpp
    LIRCodegenAddressed.cpp
    LIRCodegenDefn.cpp
    LIRCodegenOp.cpp
    LIRCodegenStmt.cpp
    LIRCodegenValued.cpp
)

add_library(Codegen
    STATIC
        ${CODEGEN_SOURCES}
)

target_include_directories(Codegen PUBLIC
    ${CMAKE_SOURCE_DIR}/lir/include/
)

target_link_libraries(Codegen
    PRIVATE
        Core
        Tree
        LIR
)
