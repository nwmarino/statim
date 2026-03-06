set(CORE_SOURCES
    Context.cpp
    Diagnostics.cpp
    ThreadPool.cpp
)

add_library(Core
    STATIC
        ${CORE_SOURCES}
)

target_link_libraries(Core
    PRIVATE
        Tools
)
