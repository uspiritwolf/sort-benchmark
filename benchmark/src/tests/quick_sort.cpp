#include <core/memory.hpp>

#include <benchmark/benchmark.h>

#include <algorithm>
#include <iterator>
#include <list>
#include <random>
#include <ranges>
#include <vector>

namespace
{
	template <typename Iter>
	void quick_sort_impl(Iter first, Iter last, std::iter_difference_t<Iter> size)
	{
		struct Partition
		{
			Iter first;
			Iter last;
			std::iter_difference_t<Iter> size;
		};

		std::vector<Partition> pending;
		for (;;)
		{
			while (size > 1)
			{
				const auto pivot = *std::next(first, size / 2);
				Iter lower = first;
				Iter current = first;
				Iter upper = last;
				std::iter_difference_t<Iter> lower_size = 0;
				std::iter_difference_t<Iter> upper_size = 0;

				while (current != upper)
				{
					if (*current < pivot)
					{
						std::iter_swap(lower, current);
						++lower;
						++current;
						++lower_size;
					}
					else if (pivot < *current)
					{
						--upper;
						std::iter_swap(current, upper);
						++upper_size;
					}
					else
					{
						++current;
					}
				}

				// Process the smaller partition first to bound pending stack size.
				if (lower_size < upper_size)
				{
					if (upper_size > 1)
					{
						pending.push_back({upper, last, upper_size});
					}
					last = lower;
					size = lower_size;
				}
				else
				{
					if (lower_size > 1)
					{
						pending.push_back({first, lower, lower_size});
					}
					first = upper;
					size = upper_size;
				}
			}

			if (pending.empty())
			{
				break;
			}

			const auto partition = pending.back();
			pending.pop_back();
			first = partition.first;
			last = partition.last;
			size = partition.size;
		}
	}

	template <typename Iter>
	void quick_sort(Iter first, Iter last)
	{
		quick_sort_impl(first, last, std::distance(first, last));
	}

	void run_quick_sort_vector(benchmark::State& state, const std::vector<int>& data)
	{
		for (auto _ : state)
		{
			state.PauseTiming();
			std::vector<int> data_copy(data);
			state.ResumeTiming();

			mem_begin_measuring_section();

			quick_sort(data_copy.begin(), data_copy.end());

			mem_end_measuring_section();
		}

		fill_memory_usage(state, benchmark::Counter::kAvgIterations);
	}

	void quick_sort_vector(benchmark::State& state)
	{
		std::mt19937 rng(std::random_device{}());

		std::vector<int> data(state.range(0));
		std::ranges::generate(data, rng);

		run_quick_sort_vector(state, data);
	}

	void quick_sort_vector_sorted(benchmark::State& state)
	{
		std::vector<int> data(state.range(0));
		std::ranges::iota(data, 0);

		run_quick_sort_vector(state, data);
	}

	void quick_sort_vector_reverse_sorted(benchmark::State& state)
	{
		std::vector<int> data(state.range(0));
		std::ranges::iota(std::views::reverse(data), 0);

		run_quick_sort_vector(state, data);
	}

	void quick_sort_list(benchmark::State& state)
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

			quick_sort(data_copy.begin(), data_copy.end());

			mem_end_measuring_section();
		}

		fill_memory_usage(state, benchmark::Counter::kAvgIterations);
	}

	BENCHMARK(quick_sort_vector)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6);

	BENCHMARK(quick_sort_vector_sorted)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6);

	BENCHMARK(quick_sort_vector_reverse_sorted)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6);

	BENCHMARK(quick_sort_list)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6);

} // namespace
