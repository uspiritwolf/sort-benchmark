#include <core/memory.hpp>

#include <benchmark/benchmark.h>

#include <algorithm>
#include <cassert>
#include <iterator>
#include <list>
#include <random>
#include <ranges>
#include <utility>
#include <vector>

namespace
{
	template <typename Iter>
	void insertion_sort(Iter first, Iter last)
	{
		if (first == last)
		{
			return;
		}

		for (Iter i = std::next(first); i != last; ++i)
		{
			auto value = std::move(*i);
			Iter position = i;
			while (position != first)
			{
				Iter previous = std::prev(position);
				if (!(value < *previous))
				{
					break;
				}
				*position = std::move(*previous);
				position = previous;
			}
			*position = std::move(value);
		}
	}

	void run_insertion_sort_vector(benchmark::State& state, const std::vector<int>& data)
	{
		for (auto _ : state)
		{
			state.PauseTiming();
			std::vector<int> data_copy(data);
			state.ResumeTiming();

			mem_begin_measuring_section();

			insertion_sort(data_copy.begin(), data_copy.end());
			assert(std::ranges::is_sorted(data_copy));

			mem_end_measuring_section();
		}

		fill_memory_usage(state, benchmark::Counter::kAvgIterations);
	}

	void insertion_sort_vector(benchmark::State& state)
	{
		std::mt19937 rng(std::random_device{}());

		std::vector<int> data(state.range(0));
		std::ranges::generate(data, rng);

		run_insertion_sort_vector(state, data);
	}

	void insertion_sort_vector_sorted(benchmark::State& state)
	{
		std::vector<int> data(state.range(0));
		std::ranges::iota(data, 0);

		run_insertion_sort_vector(state, data);
	}

	void insertion_sort_vector_reverse_sorted(benchmark::State& state)
	{
		std::vector<int> data(state.range(0));
		std::ranges::iota(std::views::reverse(data), 0);

		run_insertion_sort_vector(state, data);
	}

	void insertion_sort_list(benchmark::State& state)
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

			insertion_sort(data_copy.begin(), data_copy.end());
			assert(std::ranges::is_sorted(data_copy));

			mem_end_measuring_section();
		}

		fill_memory_usage(state, benchmark::Counter::kAvgIterations);
	}

	BENCHMARK(insertion_sort_vector)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6);

	BENCHMARK(insertion_sort_vector_sorted)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6);

	BENCHMARK(insertion_sort_vector_reverse_sorted)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6);

	BENCHMARK(insertion_sort_list)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6);

} // namespace
