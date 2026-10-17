#include <benchmark/benchmark.h>

#include <algorithm>
#include <cassert>
#include <iterator>
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

	void run_shell_sort(benchmark::State& state, const std::vector<int>& data)
	{
		for (auto _ : state)
		{
			state.PauseTiming();
			std::vector<int> data_copy(data);
			state.ResumeTiming();

			shell_sort(data_copy.begin(), data_copy.end());
			assert(std::ranges::is_sorted(data_copy));
		}

		state.SetComplexityN(state.range(0));
	}

	void shell_sort_random(benchmark::State& state)
	{
		std::mt19937 rng(std::random_device{}());

		std::vector<int> data(state.range(0));
		std::ranges::generate(data, rng);

		run_shell_sort(state, data);
	}

	void shell_sort_sorted(benchmark::State& state)
	{
		std::vector<int> data(state.range(0));
		std::ranges::iota(data, 0);

		run_shell_sort(state, data);
	}

	void shell_sort_reverse_sorted(benchmark::State& state)
	{
		std::vector<int> data(state.range(0));
		std::ranges::iota(std::views::reverse(data), 0);

		run_shell_sort(state, data);
	}

	BENCHMARK(shell_sort_random)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6)
		->Complexity();

	BENCHMARK(shell_sort_sorted)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6)
		->Complexity();

	BENCHMARK(shell_sort_reverse_sorted)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6)
		->Complexity();

} // namespace
