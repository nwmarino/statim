set(MACHINE_SOURCES
    Machine.cpp
    MachineFunction.cpp
    MachineLabel.cpp
    MachineObject.cpp
    MachineOp.cpp
)

add_library(Machine
    STATIC
        ${MACHINE_SOURCES}
)

target_link_libraries(Machine
    PRIVATE
        Graph
)
