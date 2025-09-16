add_library(core
    STATIC
        lexer.cpp
        logger.cpp
)

target_link_libraries(core
    PRIVATE
        types
)
