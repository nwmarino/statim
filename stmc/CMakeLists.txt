cmake_minimum_required(VERSION 3.20)
project(statim VERSION 0.1.0 LANGUAGES C CXX)

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

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

message(STATUS "LLVM includes: ${LLVM_INCLUDE_DIRS}")
message(STATUS "LLVM definitions: ${LLVM_DEFINITIONS}")
message(STATUS "LLVM tools: ${LLVM_TOOLS_BINARY_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

include_directories(
    ${CMAKE_SOURCE_DIR}/source
    ${Boost_INCLUDE_DIRS}
    ${GTEST_INCLUDE_DIRS}
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(source)

add_executable(stmc source/stmc.cpp)

target_link_libraries(stmc 
    PRIVATE
        ${Boost_LIBRARIES}
        core
        siir
        tree
        types
        x64
)

target_include_directories(stmc
    PUBLIC
        ${CMAKE_SOURCE_DIR}/source
)

llvm_config(stmc USE_SHARED core irreader support clang)

# Testing

enable_testing()

add_executable(stmc_test
    test/test_codegen.cpp
    test/test_index.cpp
    test/test_lexer.cpp
    test/test_parser.cpp
    test/test_syma.cpp
    test/test_sema.cpp
    test/test_x64.cpp
)

target_link_libraries(stmc_test 
    PRIVATE
        ${Boost_LIBRARIES}
        GTest::GTest
        GTest::Main
        core
        siir
        tree
        types
        x64
)

llvm_config(stmc_test USE_SHARED core irreader support clang)

add_test(
    NAME stmc_test
    COMMAND stmc_test
)

set(GTEST_OUTPUT "xml:${CMAKE_BINARY_DIR}/test_results.xml")
