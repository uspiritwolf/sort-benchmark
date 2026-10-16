#include <benchmark/benchmark.h>

#include <algorithm>
#include <cassert>
#include <iterator>
#include <random>
#include <ranges>
#include <vector>

namespace
{
	template <typename Iter>
	void selection_sort(Iter first, Iter last)
	{
		for (Iter i = first; i != last; ++i)
		{
			Iter min = i;
			for (Iter j = std::next(i); j != last; ++j)
			{
				if (*j < *min)
				{
					min = j;
				}
			}

			if (min != i)
			{
				std::iter_swap(i, min);
			}
		}
	}

	void run_selection_sort_vector(benchmark::State& state, const std::vector<int>& data)
	{
		for (auto _ : state)
		{
			state.PauseTiming();
			std::vector<int> data_copy(data);
			state.ResumeTiming();

			selection_sort(data_copy.begin(), data_copy.end());
			assert(std::ranges::is_sorted(data_copy));
		}

		state.SetComplexityN(state.range(0));
	}

	void selection_sort_vector(benchmark::State& state)
	{
		std::mt19937 rng(std::random_device{}());

		std::vector<int> data(state.range(0));
		std::ranges::generate(data, rng);

		run_selection_sort_vector(state, data);
	}

	void selection_sort_vector_sorted(benchmark::State& state)
	{
		std::vector<int> data(state.range(0));
		std::ranges::iota(data, 0);

		run_selection_sort_vector(state, data);
	}

	void selection_sort_vector_reverse_sorted(benchmark::State& state)
	{
		std::vector<int> data(state.range(0));
		std::ranges::iota(std::views::reverse(data), 0);

		run_selection_sort_vector(state, data);
	}

	BENCHMARK(selection_sort_vector)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6)
		->Complexity();

	BENCHMARK(selection_sort_vector_sorted)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6)
		->Complexity();

	BENCHMARK(selection_sort_vector_reverse_sorted)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6)
		->Complexity();

} // namespace
