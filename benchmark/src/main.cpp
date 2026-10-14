#include <benchmark/benchmark.h>

#include "core/memory.hpp"

auto main(int argc, char** argv) -> int
{

	benchmark::MaybeReenterWithoutASLR(argc, argv);

	benchmark::RegisterMemoryManager(get_default_mem_mgr());

	benchmark::Initialize(&argc, argv);

	if (benchmark::ReportUnrecognizedArguments(argc, argv))
		return 1;

	benchmark::RunSpecifiedBenchmarks();
	benchmark::Shutdown();

	return 0;
}
