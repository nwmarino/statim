set(MACHINE_SOURCES
    Machine.cpp
)

add_library(Machine
    STATIC
        ${MACHINE_SOURCES}
)

target_link_libraries(Machine
    PRIVATE
        Graph
)
