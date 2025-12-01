cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0135 NEW)

project(IntegralCommunication
    VERSION 1.0.0
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Detect if this project is the top-level one
set(INTEGRALCOMM_IS_TOP_LEVEL OFF)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(INTEGRALCOMM_IS_TOP_LEVEL ON)
endif()

option(INTEGRALCOMM_BUILD_TESTS "Build IntegralCommunication tests" ${INTEGRALCOMM_IS_TOP_LEVEL})

# Collect all source files from src/
file(GLOB_RECURSE INTEGRALCOMM_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.[ch]
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.[ch]pp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.[ch]xx
)

# Main library target
add_library(IntegralCommunication ${INTEGRALCOMM_SOURCES})
add_library(IntegralCommunication::IntegralCommunication ALIAS IntegralCommunication)

target_include_directories(IntegralCommunication
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(IntegralCommunication PUBLIC cxx_std_17)

# ----------------- Tests -----------------
if(INTEGRALCOMM_BUILD_TESTS)
    include(FetchContent)

    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )

    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    file(GLOB_RECURSE INTEGRALCOMM_TEST_SOURCES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp
    )

    add_executable(IntegralCommunicationTests ${INTEGRALCOMM_TEST_SOURCES})

    target_link_libraries(IntegralCommunicationTests PRIVATE
        IntegralCommunication::IntegralCommunication
        GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(IntegralCommunicationTests)
endif()
