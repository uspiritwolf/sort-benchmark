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
	void gnome_sort(Iter first, Iter last)
	{
		Iter current = first;
		while (current != last)
		{
			if (current == first)
			{
				++current;
			}
			else
			{
				Iter previous = std::prev(current);
				if (*current < *previous)
				{
					std::iter_swap(current, previous);
					current = previous;
				}
				else
				{
					++current;
				}
			}
		}
	}

	void run_gnome_sort(benchmark::State& state, const std::vector<int>& data)
	{
		for (auto _ : state)
		{
			state.PauseTiming();
			std::vector<int> data_copy(data);
			state.ResumeTiming();

			gnome_sort(data_copy.begin(), data_copy.end());
			assert(std::ranges::is_sorted(data_copy));
		}

		state.SetComplexityN(state.range(0));
	}

	void gnome_sort_random(benchmark::State& state)
	{
		std::mt19937 rng(std::random_device{}());

		std::vector<int> data(state.range(0));
		std::ranges::generate(data, rng);

		run_gnome_sort(state, data);
	}

	void gnome_sort_sorted(benchmark::State& state)
	{
		std::vector<int> data(state.range(0));
		std::ranges::iota(data, 0);

		run_gnome_sort(state, data);
	}

	void gnome_sort_reverse_sorted(benchmark::State& state)
	{
		std::vector<int> data(state.range(0));
		std::ranges::iota(std::views::reverse(data), 0);

		run_gnome_sort(state, data);
	}

	BENCHMARK(gnome_sort_random)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6)
		->Complexity();

	BENCHMARK(gnome_sort_sorted)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6)
		->Complexity();

	BENCHMARK(gnome_sort_reverse_sorted)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6)
		->Complexity();

} // namespace
