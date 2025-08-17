cmake_minimum_required(VERSION 3.20)
project(statim VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)

cmake_policy(SET CMP0167 OLD)

find_package(LLVM REQUIRED CONFIG)
find_package(Boost REQUIRED COMPONENTS filesystem system)
find_package(GTest REQUIRED)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

include_directories(
    ${CMAKE_SOURCE_DIR}/compiler/include
    ${Boost_INCLUDE_DIRS}
    ${GTEST_INCLUDE_DIRS}
)

set(COMPILER_SOURCES
    compiler/src/ast.cpp
    compiler/src/ast_print.cpp
    compiler/src/bytecode.cpp
    compiler/src/input_file.cpp
    compiler/src/lexer.cpp
    compiler/src/logger.cpp
    compiler/src/parser.cpp
    compiler/src/scope.cpp
    compiler/src/sema.cpp
    compiler/src/syma.cpp
    compiler/src/token.cpp
    compiler/src/type.cpp
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(stmc
    compiler/stmc.cpp
    ${COMPILER_SOURCES}
)

target_link_libraries(stmc PRIVATE
    ${LLVM_LIBRARIES}
    ${Boost_LIBRARIES}
)

enable_testing()

add_executable(stmc_test
    test/test_index.cpp
    test/test_lexer.cpp
    test/test_parser.cpp
    ${COMPILER_SOURCES}
)

target_link_libraries(stmc_test PRIVATE
    ${LLVM_LIBRARIES}
    ${Boost_LIBRARIES}
    GTest::GTest
    GTest::Main
)

add_test(
    NAME stmc_test
    COMMAND stmc_test
)

set(GTEST_OUTPUT "xml:${CMAKE_BINARY_DIR}/test_results.xml")
