cmake_minimum_required(VERSION 4.2 FATAL_ERROR)

#ENV
set(CMAKE_COLOR_DIAGNOSTICS ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

#IDA
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project (BenchmarkApp VERSION 0.0.1)

# Deps config
set(FETCHCONTENT_QUIET OFF)
include(FetchContent)
include(cmake/google_benchmark.cmake)

# Projects
add_subdirectory(benchmark)


# Install
install(TARGETS benchmark_tool DESTINATION . COMPONENT runtime)
