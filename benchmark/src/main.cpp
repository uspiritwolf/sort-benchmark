#include <benchmark/benchmark.h>

auto main(int argc, char** argv) -> int
{
	benchmark::MaybeReenterWithoutASLR(argc, argv);

	benchmark::Initialize(&argc, argv);

	if (benchmark::ReportUnrecognizedArguments(argc, argv))
		return 1;

	benchmark::RunSpecifiedBenchmarks();
	benchmark::Shutdown();

	return 0;
}
