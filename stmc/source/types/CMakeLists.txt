add_library(types
    STATIC
        input_file.cpp
        token.cpp
)

target_link_libraries(types
    PRIVATE
        core
        siir
        tree
)
