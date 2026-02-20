set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(source)

add_executable(lace source/lace.cpp)

target_compile_options(lace PRIVATE -g)

target_link_libraries(lace PUBLIC
    Codegen
    Core
    Lexer
    LIR
    Parser
    Tree
    Tools
)

target_include_directories(Codegen PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_include_directories(Core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_include_directories(Lexer PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_include_directories(Tree PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_include_directories(lace PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(lace PUBLIC cxx_std_20)

add_executable(lace_test
    test/LexerTests.cpp
    test/DefnParserTests.cpp
    test/ExprParserTests.cpp
    test/RuneParserTests.cpp
    test/StmtParserTests.cpp
    test/TypeParserTests.cpp
    test/SymbolAnalysisTests.cpp
    test/SemanticAnalysisTests.cpp
    test/CodegenTests.cpp
)

target_include_directories(lace_test PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(lace_test PRIVATE
    Codegen
    Core
    Lexer
    LIR
    Parser
    Tree
    Tools
    GTest::GTest
    GTest::Main
)
