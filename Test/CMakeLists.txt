cmake_minimum_required(VERSION 3.10)
project(Ground_Block_Tests C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Unity Test Framework setup
set(UNITY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Unity)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/Unity/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../APL/Tasks/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../Motor/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../FML/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Source files for Unity framework
set(UNITY_SOURCES
    ${UNITY_DIR}/src/unity.c
)

# Source files for the module under test
set(MODULE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../APL/Tasks/Src/Ground_Block.c
)

# Test source files
set(TEST_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/test_Ground_Block.c
)

# Create test executable
add_executable(test_Ground_Block
    ${UNITY_SOURCES}
    ${MODULE_SOURCES}
    ${TEST_SOURCES}
)

# Enable testing
enable_testing()

# Add test
add_test(NAME Ground_Block_Tests COMMAND test_Ground_Block)

# Compiler warnings
if(MSVC)
    target_compile_options(test_Ground_Block PRIVATE /W4)
else()
    target_compile_options(test_Ground_Block PRIVATE -Wall -Wextra -pedantic)
endif()

# Print configuration summary
message(STATUS "=========================================")
message(STATUS "Ground_Block Unit Tests Configuration")
message(STATUS "=========================================")
message(STATUS "C Standard: ${CMAKE_C_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Test Executable: test_Ground_Block")
message(STATUS "=========================================")