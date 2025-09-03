cmake_minimum_required(VERSION 3.20)
project(statim VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)

cmake_policy(SET CMP0167 OLD)

#find_package(LLVM REQUIRED CONFIG)
find_package(Boost REQUIRED COMPONENTS filesystem system)
find_package(GTest REQUIRED)

#include_directories(${LLVM_INCLUDE_DIRS})
#add_definitions(${LLVM_DEFINITIONS})

include_directories(
    ${CMAKE_SOURCE_DIR}/source
    ${Boost_INCLUDE_DIRS}
    ${GTEST_INCLUDE_DIRS}
)

set(COMPILER_SOURCES
    source/core/lexer.cpp
    source/core/logger.cpp
    source/core/type.cpp
    source/siir/basicblock.cpp
    source/siir/cfg.cpp
    source/siir/constant.cpp
    source/siir/function.cpp
    source/siir/global.cpp
    source/siir/instbuilder.cpp
    source/siir/instruction.cpp
    source/siir/local.cpp
    source/siir/print.cpp
    source/siir/ssa_rewrite_pass.cpp
    source/siir/target.cpp
    source/siir/trivial_dce_pass.cpp
    source/siir/type.cpp
    source/siir/value.cpp
    source/tree/codegen.cpp
    source/tree/decl.cpp
    source/tree/expr.cpp
    source/tree/parser.cpp
    source/tree/print.cpp
    source/tree/root.cpp
    source/tree/rune.cpp
    source/tree/scope.cpp
    source/tree/sema.cpp
    source/tree/stmt.cpp
    source/tree/syma.cpp
    source/types/input_file.cpp
    source/types/token.cpp
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(stmc
    source/stmc.cpp
    ${COMPILER_SOURCES}
)

target_link_libraries(stmc PRIVATE
#    ${LLVM_LIBRARIES}
    ${Boost_LIBRARIES}
)

enable_testing()

add_executable(stmc_test
    test/test_index.cpp
    test/test_lexer.cpp
    test/test_parser.cpp
    test/test_syma.cpp
    test/test_sema.cpp
    ${COMPILER_SOURCES}
)

target_link_libraries(stmc_test PRIVATE
 #   ${LLVM_LIBRARIES}
    ${Boost_LIBRARIES}
    GTest::GTest
    GTest::Main
)

add_test(
    NAME stmc_test
    COMMAND stmc_test
)

set(GTEST_OUTPUT "xml:${CMAKE_BINARY_DIR}/test_results.xml")
