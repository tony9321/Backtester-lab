cmake_minimum_required(VERSION 3.20)
project(quantlab VERSION 1.0.0 LANGUAGES CXX)

# Modern C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags for performance
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wpedantic")

# Dependencies management with FetchContent
include(FetchContent)

# nlohmann JSON for config and output
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)

# fmt for fast formatting and logging
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 10.1.1
)

# cpr for HTTP requests (Alpaca API)
FetchContent_Declare(
    cpr
    GIT_REPOSITORY https://github.com/libcpr/cpr.git
    GIT_TAG 1.10.5
)

# Catch2 for unit testing
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.4.0
)

FetchContent_MakeAvailable(nlohmann_json fmt cpr Catch2)

# Core library
add_subdirectory(src)

# Applications
add_subdirectory(apps)

# Tests removed - focusing on core functionality