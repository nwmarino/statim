set(ANALYSIS_SOURCES
    AMD64LoweringPass.cpp
    LoweringPass.cpp
    #SSARewritePass.cpp
)

add_library(Analysis
    STATIC
        ${ANALYSIS_SOURCES}
)

target_link_libraries(Analysis
    PRIVATE
        Graph
        Machine
)
