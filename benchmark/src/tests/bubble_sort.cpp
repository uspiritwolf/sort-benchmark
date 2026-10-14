
#include <benchmark/benchmark.h>

#include <algorithm>
#include <random>

#include <core/memory.hpp>

namespace
{
	template <typename Iter>
	void bubble_sort(Iter first, Iter last)
	{
		for (Iter i = first; i != last; ++i)
		{
			for (Iter j = first; j < i; ++j)
			{
				if (*i < *j)
				{
					std::iter_swap(i, j);
				}
			}
		}
	}

	void bubble_sort_entry(benchmark::State& state)
	{
		std::mt19937 rng(std::random_device{}());

		for (auto _ : state)
		{
			state.PauseTiming();
			std::vector<int> data(state.range(0));
			std::ranges::generate(data, rng);
			state.ResumeTiming();

			mem_begin_measuring_section();

			bubble_sort(data.begin(), data.end());

			mem_end_measuring_section();
		}

		fill_memory_usage(state, benchmark::Counter::kAvgIterations);
	}

	BENCHMARK(bubble_sort_entry)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6);

} // namespace
