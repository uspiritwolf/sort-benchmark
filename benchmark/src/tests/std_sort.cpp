#include <benchmark/benchmark.h>

#include <algorithm>
#include <cassert>
#include <list>
#include <numeric>
#include <random>
#include <ranges>
#include <vector>

namespace
{
	void run_std_sort_vector(benchmark::State& state, const std::vector<int>& data)
	{
		for (auto _ : state)
		{
			state.PauseTiming();
			std::vector<int> data_copy(data);
			state.ResumeTiming();

			std::ranges::sort(data_copy);
			assert(std::ranges::is_sorted(data_copy));
		}

		state.SetComplexityN(state.range(0));
	}

	void std_sort_vector(benchmark::State& state)
	{
		std::mt19937 rng(std::random_device{}());

		std::vector<int> data(state.range(0));
		std::ranges::generate(data, rng);

		run_std_sort_vector(state, data);
	}

	void std_sort_vector_sorted(benchmark::State& state)
	{
		std::vector<int> data(state.range(0));
		std::ranges::iota(data, 0);

		run_std_sort_vector(state, data);
	}

	void std_sort_vector_reverse_sorted(benchmark::State& state)
	{
		std::vector<int> data(state.range(0));
		std::ranges::iota(std::views::reverse(data), 0);

		run_std_sort_vector(state, data);
	}

	void std_sort_list(benchmark::State& state)
	{
		std::mt19937 rng(std::random_device{}());

		std::list<int> origin_data(state.range(0));
		std::ranges::generate(origin_data, rng);

		for (auto _ : state)
		{
			state.PauseTiming();
			std::list<int> data_copy(origin_data);
			state.ResumeTiming();

			data_copy.sort();
			assert(std::ranges::is_sorted(data_copy));
		}

		state.SetComplexityN(state.range(0));
	}

	BENCHMARK(std_sort_vector)
		->RangeMultiplier(2)
		->Range(128, 65536)
		->Complexity();

	BENCHMARK(std_sort_vector_sorted)
		->RangeMultiplier(2)
		->Range(128, 65536)
		->Complexity();

	BENCHMARK(std_sort_vector_reverse_sorted)
		->RangeMultiplier(2)
		->Range(128, 65536)
		->Complexity();

	BENCHMARK(std_sort_list)
		->RangeMultiplier(2)
		->Range(128, 65536)
		->Complexity();

} // namespace
