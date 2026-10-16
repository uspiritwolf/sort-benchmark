#include <core/memory.hpp>

#include <benchmark/benchmark.h>

#include <algorithm>
#include <cassert>
#include <numeric>
#include <random>
#include <ranges>
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

	void run_bubble_sort_vector(benchmark::State& state, const std::vector<int>& data)
	{
		for (auto _ : state)
		{
			state.PauseTiming();
			std::vector<int> data_copy(data);
			state.ResumeTiming();

			mem_begin_measuring_section();

			bubble_sort(data_copy.begin(), data_copy.end());
			assert(std::ranges::is_sorted(data_copy));

			mem_end_measuring_section();
		}

		fill_memory_usage(state, benchmark::Counter::kAvgIterations);
	}

	void bubble_sort_vector(benchmark::State& state)
	{
		std::mt19937 rng(std::random_device{}());

		std::vector<int> data(state.range(0));
		std::ranges::generate(data, rng);

		run_bubble_sort_vector(state, data);
	}

	void bubble_sort_vector_sorted(benchmark::State& state)
	{
		std::vector<int> data(state.range(0));
		std::ranges::iota(data, 0);

		run_bubble_sort_vector(state, data);
	}

	void bubble_sort_vector_reverse_sorted(benchmark::State& state)
	{
		std::vector<int> data(state.range(0));
		std::ranges::iota(std::views::reverse(data), 0);

		run_bubble_sort_vector(state, data);
	}

	BENCHMARK(bubble_sort_vector)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6);

	BENCHMARK(bubble_sort_vector_sorted)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6);

	BENCHMARK(bubble_sort_vector_reverse_sorted)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6);

} // namespace
