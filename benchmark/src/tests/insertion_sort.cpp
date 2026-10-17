#include <benchmark/benchmark.h>

#include <algorithm>
#include <cassert>
#include <iterator>
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

	void run_insertion_sort(benchmark::State& state, const std::vector<int>& data)
	{
		for (auto _ : state)
		{
			state.PauseTiming();
			std::vector<int> data_copy(data);
			state.ResumeTiming();

			insertion_sort(data_copy.begin(), data_copy.end());
			assert(std::ranges::is_sorted(data_copy));
		}

		state.SetComplexityN(state.range(0));
	}

	void insertion_sort_random(benchmark::State& state)
	{
		std::mt19937 rng(std::random_device{}());

		std::vector<int> data(state.range(0));
		std::ranges::generate(data, rng);

		run_insertion_sort(state, data);
	}

	void insertion_sort_sorted(benchmark::State& state)
	{
		std::vector<int> data(state.range(0));
		std::ranges::iota(data, 0);

		run_insertion_sort(state, data);
	}

	void insertion_sort_reverse_sorted(benchmark::State& state)
	{
		std::vector<int> data(state.range(0));
		std::ranges::iota(std::views::reverse(data), 0);

		run_insertion_sort(state, data);
	}

	BENCHMARK(insertion_sort_random)
		->RangeMultiplier(2)
		->Range(128, 4096)
		->Complexity();

	BENCHMARK(insertion_sort_sorted)
		->RangeMultiplier(2)
		->Range(128, 65536)
		->Complexity();

	BENCHMARK(insertion_sort_reverse_sorted)
		->RangeMultiplier(2)
		->Range(128, 4096)
		->Complexity();

} // namespace
