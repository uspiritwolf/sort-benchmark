#include <core/memory.hpp>

#include <benchmark/benchmark.h>

#include <algorithm>
#include <list>
#include <random>
#include <vector>

namespace
{
	template <typename Iter>
	void bubble_sort(Iter first, Iter last)
	{
		for (Iter i = first; i != last; ++i)
		{
			for (Iter j = first; j != i; ++j)
			{
				if (*i < *j)
				{
					std::iter_swap(i, j);
				}
			}
		}
	}

	void bubble_sort_vector(benchmark::State& state)
	{
		std::mt19937 rng(std::random_device{}());

		std::vector<int> data(state.range(0));
		std::ranges::generate(data, rng);

		for (auto _ : state)
		{
			state.PauseTiming();
			std::vector<int> data_copy(data);
			state.ResumeTiming();

			mem_begin_measuring_section();

			bubble_sort(data_copy.begin(), data_copy.end());

			mem_end_measuring_section();
		}

		fill_memory_usage(state, benchmark::Counter::kAvgIterations);
	}

	void bubble_sort_list(benchmark::State& state)
	{
		std::mt19937 rng(std::random_device{}());

		std::list<int> origin_data(state.range(0));
		std::ranges::generate(origin_data, rng);

		for (auto _ : state)
		{
			state.PauseTiming();
			std::list<int> data_copy(origin_data);
			state.ResumeTiming();

			mem_begin_measuring_section();

			bubble_sort(data_copy.begin(), data_copy.end());

			mem_end_measuring_section();
		}

		fill_memory_usage(state, benchmark::Counter::kAvgIterations);
	}

	BENCHMARK(bubble_sort_vector)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6);

	BENCHMARK(bubble_sort_list)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6);

} // namespace
