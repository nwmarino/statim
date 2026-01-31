set(MACHINE_SOURCES
    #AsmWriter.cpp
    #InstSelector.cpp
    MachFunction.cpp
    Machine.cpp
    MachInst.cpp
    MachLabel.cpp
    MachOperand.cpp
    Printer.cpp
    Register.cpp
    RegisterAllocator.cpp
    RegisterAnalysis.cpp
    Segment.cpp
)

add_library(Machine
    STATIC
        ${MACHINE_SOURCES}
)

target_link_libraries(Machine
    PRIVATE
        Graph
)
