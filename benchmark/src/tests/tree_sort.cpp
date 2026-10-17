#include <benchmark/benchmark.h>

#include <algorithm>
#include <cassert>
#include <iterator>
#include <random>
#include <ranges>
#include <set>
#include <vector>

namespace
{
	template <typename Iter>
	void tree_sort(Iter first, Iter last)
	{
		std::multiset<std::iter_value_t<Iter>> tree(first, last);
		std::ranges::copy(tree, first);
	}

	void run_tree_sort(benchmark::State& state, const std::vector<int>& data)
	{
		for (auto _ : state)
		{
			state.PauseTiming();
			std::vector<int> data_copy(data);
			state.ResumeTiming();

			tree_sort(data_copy.begin(), data_copy.end());
			assert(std::ranges::is_sorted(data_copy));
		}

		state.SetComplexityN(state.range(0));
	}

	void tree_sort_random(benchmark::State& state)
	{
		std::mt19937 rng(std::random_device{}());

		std::vector<int> data(state.range(0));
		std::ranges::generate(data, rng);

		run_tree_sort(state, data);
	}

	void tree_sort_sorted(benchmark::State& state)
	{
		std::vector<int> data(state.range(0));
		std::ranges::iota(data, 0);

		run_tree_sort(state, data);
	}

	void tree_sort_reverse_sorted(benchmark::State& state)
	{
		std::vector<int> data(state.range(0));
		std::ranges::iota(std::views::reverse(data), 0);

		run_tree_sort(state, data);
	}

	BENCHMARK(tree_sort_random)
		->RangeMultiplier(2)
		->Range(128, 65536)
		->Complexity();

	BENCHMARK(tree_sort_sorted)
		->RangeMultiplier(2)
		->Range(128, 65536)
		->Complexity();

	BENCHMARK(tree_sort_reverse_sorted)
		->RangeMultiplier(2)
		->Range(128, 65536)
		->Complexity();

} // namespace
