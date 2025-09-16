add_library(x64
    STATIC
        asm_writer.cpp
        inst_selection.cpp
        printer.cpp
        x64.cpp
)

target_link_libraries(x64
    PRIVATE
        core
        siir
        types
)
