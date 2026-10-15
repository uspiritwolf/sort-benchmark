#include <core/memory.hpp>

#include <benchmark/benchmark.h>

#include <algorithm>
#include <iterator>
#include <list>
#include <numeric>
#include <random>
#include <ranges>
#include <utility>
#include <vector>

namespace
{
	template <typename Iter>
	void shell_sort(Iter first, Iter last)
	{
		const auto size = std::distance(first, last);
		for (auto gap = size / 2; gap > 0; gap /= 2)
		{
			auto index = gap;
			for (Iter i = std::next(first, gap); i != last; ++i, ++index)
			{
				auto value = std::move(*i);
				Iter position = i;
				auto position_index = index;
				while (position_index >= gap)
				{
					Iter previous = std::prev(position, gap);
					if (!(value < *previous))
					{
						break;
					}
					*position = std::move(*previous);
					position = previous;
					position_index -= gap;
				}
				*position = std::move(value);
			}
		}
	}

	void run_shell_sort_vector(benchmark::State& state, const std::vector<int>& data)
	{
		for (auto _ : state)
		{
			state.PauseTiming();
			std::vector<int> data_copy(data);
			state.ResumeTiming();

			mem_begin_measuring_section();

			shell_sort(data_copy.begin(), data_copy.end());

			mem_end_measuring_section();
		}

		fill_memory_usage(state, benchmark::Counter::kAvgIterations);
	}

	void shell_sort_vector(benchmark::State& state)
	{
		std::mt19937 rng(std::random_device{}());

		std::vector<int> data(state.range(0));
		std::ranges::generate(data, rng);

		run_shell_sort_vector(state, data);
	}

	void shell_sort_vector_sorted(benchmark::State& state)
	{
		std::vector<int> data(state.range(0));
		std::ranges::iota(data, 0);

		run_shell_sort_vector(state, data);
	}

	void shell_sort_vector_reverse_sorted(benchmark::State& state)
	{
		std::vector<int> data(state.range(0));
		std::ranges::iota(std::views::reverse(data), 0);

		run_shell_sort_vector(state, data);
	}

	void shell_sort_list(benchmark::State& state)
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

			shell_sort(data_copy.begin(), data_copy.end());

			mem_end_measuring_section();
		}

		fill_memory_usage(state, benchmark::Counter::kAvgIterations);
	}

	BENCHMARK(shell_sort_vector)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6);

	BENCHMARK(shell_sort_vector_sorted)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6);

	BENCHMARK(shell_sort_vector_reverse_sorted)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6);

	BENCHMARK(shell_sort_list)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6);

} // namespace
