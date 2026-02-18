set(MACHINE_SOURCES
    AMD64.cpp
    AsmWriter.cpp
    CallsiteAnalysis.cpp
    FunctionABI.cpp
    LinearScan.cpp
    Machine.cpp
    MachineFunction.cpp
    MachineLabel.cpp
    MachineObject.cpp
    MachineOp.cpp
    Printer.cpp
    RegisterAllocator.cpp
    RegisterAnalysis.cpp
)

add_library(Machine
    STATIC
        ${MACHINE_SOURCES}
)

target_link_libraries(Machine
    PRIVATE
        Graph
)
