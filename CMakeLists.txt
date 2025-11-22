cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0135 NEW)
project(SevenBitEncoding LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---- Collect sources (everything under src) ----
file(GLOB_RECURSE SEVENBITENCODING_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.[ch]
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.[ch]pp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.[ch]xx
)

# ---- Library ----
add_library(SevenBitEncoding ${SEVENBITENCODING_SOURCES})
target_include_directories(SevenBitEncoding PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_features(SevenBitEncoding PUBLIC cxx_std_17)

# ---- GoogleTest ----
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp
)
add_executable(TESTS ${TEST_SOURCES})

target_include_directories(TESTS PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/tests
)

target_link_libraries(TESTS PRIVATE SevenBitEncoding GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(TESTS)
