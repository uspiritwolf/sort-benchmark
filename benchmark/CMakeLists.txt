file(
	GLOB_RECURSE
	BENCHMARK_SOURCE_FILES
	CONFIGURE_DEPENDS
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
)

add_executable(benchmark_tool)

target_sources(benchmark_tool PRIVATE ${BENCHMARK_SOURCE_FILES})
target_include_directories(benchmark_tool PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/)

# Properties
set_property(TARGET benchmark_tool PROPERTY COMPILE_WARNING_AS_ERROR ON)


# Deps
target_link_libraries(benchmark_tool PRIVATE benchmark::benchmark)
