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
	void merge_sort(std::list<T>& data)
	{
		const auto size = data.size();
		for (decltype(data.size()) width = 1; width < size;)
		{
			auto run = data.begin();
			auto remaining = size;
			while (remaining > 0)
			{
				auto left_size = std::min(width, remaining);
				auto left = run;
				auto right = std::next(left, left_size);
				remaining -= left_size;
				auto right_size = std::min(width, remaining);
				auto end = std::next(right, right_size);
				remaining -= right_size;

				while (left_size > 0 && right_size > 0)
				{
					if (*right < *left)
					{
						auto node = right++;
						data.splice(left, data, node);
						--right_size;
					}
					else
					{
						++left;
						--left_size;
					}
				}
				run = end;
			}

			if (width >= size - width)
			{
				break;
			}
			width *= 2;
		}
	}

	void run_merge_sort_list(benchmark::State& state, const std::list<int>& data)
	{
		for (auto _ : state)
		{
			state.PauseTiming();
			std::list<int> data_copy(data);
			state.ResumeTiming();

			merge_sort(data_copy);
			assert(std::ranges::is_sorted(data_copy));
		}

		state.SetComplexityN(state.range(0));
	}

	void merge_sort_list(benchmark::State& state)
	{
		std::mt19937 rng(std::random_device{}());

		std::list<int> data(state.range(0));
		std::ranges::generate(data, rng);

		run_merge_sort_list(state, data);
	}

	void merge_sort_list_sorted(benchmark::State& state)
	{
		std::list<int> data(state.range(0));
		std::ranges::iota(data, 0);

		run_merge_sort_list(state, data);
	}

	void merge_sort_list_reverse_sorted(benchmark::State& state)
	{
		std::list<int> data(state.range(0));
		std::ranges::iota(std::views::reverse(data), 0);

		run_merge_sort_list(state, data);
	}

	BENCHMARK(merge_sort_list)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6)
		->Complexity();

	BENCHMARK(merge_sort_list_sorted)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6)
		->Complexity();

	BENCHMARK(merge_sort_list_reverse_sorted)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6)
		->Complexity();

} // namespace
