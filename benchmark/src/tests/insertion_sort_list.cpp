#include <benchmark/benchmark.h>

#include <algorithm>
#include <cassert>
#include <iterator>
#include <list>
#include <numeric>
#include <random>
#include <ranges>

namespace
{
	template <typename T>
	void insertion_sort(std::list<T>& data)
	{
		if (data.empty())
		{
			return;
		}

		auto current = std::next(data.begin());
		while (current != data.end())
		{
			auto node = current++;
			auto position = node;
			while (position != data.begin())
			{
				auto previous = std::prev(position);
				if (!(*node < *previous))
				{
					break;
				}
				position = previous;
			}
			if (position != node)
			{
				data.splice(position, data, node);
			}
		}
	}

	void run_insertion_sort_list(benchmark::State& state, const std::list<int>& data)
	{
		for (auto _ : state)
		{
			state.PauseTiming();
			std::list<int> data_copy(data);
			state.ResumeTiming();

			insertion_sort(data_copy);
			assert(std::ranges::is_sorted(data_copy));
		}

		state.SetComplexityN(state.range(0));
	}

	void insertion_sort_list(benchmark::State& state)
	{
		std::mt19937 rng(std::random_device{}());

		std::list<int> data(state.range(0));
		std::ranges::generate(data, rng);

		run_insertion_sort_list(state, data);
	}

	void insertion_sort_list_sorted(benchmark::State& state)
	{
		std::list<int> data(state.range(0));
		std::ranges::iota(data, 0);

		run_insertion_sort_list(state, data);
	}

	void insertion_sort_list_reverse_sorted(benchmark::State& state)
	{
		std::list<int> data(state.range(0));
		std::ranges::iota(std::views::reverse(data), 0);

		run_insertion_sort_list(state, data);
	}

	BENCHMARK(insertion_sort_list)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6)
		->Complexity();

	BENCHMARK(insertion_sort_list_sorted)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6)
		->Complexity();

	BENCHMARK(insertion_sort_list_reverse_sorted)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6)
		->Complexity();

} // namespace
