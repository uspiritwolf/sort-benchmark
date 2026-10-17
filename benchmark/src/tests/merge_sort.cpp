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
	void merge_sort(Iter first, Iter last)
	{
		const auto size = std::distance(first, last);
		if (size < 2)
		{
			return;
		}

		std::vector<std::iter_value_t<Iter>> buffer;
		buffer.reserve(size);

		for (std::iter_difference_t<Iter> width = 1; width < size;)
		{
			Iter run = first;
			auto remaining = size;
			while (remaining > 0)
			{
				const auto left_size = std::min(width, remaining);
				Iter middle = std::next(run, left_size);
				remaining -= left_size;
				const auto right_size = std::min(width, remaining);
				Iter end = std::next(middle, right_size);
				remaining -= right_size;

				Iter left = run;
				Iter right = middle;
				while (left != middle && right != end)
				{
					if (*right < *left)
					{
						buffer.push_back(std::move(*right));
						++right;
					}
					else
					{
						buffer.push_back(std::move(*left));
						++left;
					}
				}

				while (left != middle)
				{
					buffer.push_back(std::move(*left));
					++left;
				}

				while (right != end)
				{
					buffer.push_back(std::move(*right));
					++right;
				}

				run = end;
			}

			std::move(buffer.begin(), buffer.end(), first);
			buffer.clear();

			if (width >= size - width)
			{
				break;
			}
			width *= 2;
		}
	}

	void run_merge_sort(benchmark::State& state, const std::vector<int>& data)
	{
		for (auto _ : state)
		{
			state.PauseTiming();
			std::vector<int> data_copy(data);
			state.ResumeTiming();

			merge_sort(data_copy.begin(), data_copy.end());
			assert(std::ranges::is_sorted(data_copy));
		}

		state.SetComplexityN(state.range(0));
	}

	void merge_sort_random(benchmark::State& state)
	{
		std::mt19937 rng(std::random_device{}());

		std::vector<int> data(state.range(0));
		std::ranges::generate(data, rng);

		run_merge_sort(state, data);
	}

	void merge_sort_sorted(benchmark::State& state)
	{
		std::vector<int> data(state.range(0));
		std::ranges::iota(data, 0);

		run_merge_sort(state, data);
	}

	void merge_sort_reverse_sorted(benchmark::State& state)
	{
		std::vector<int> data(state.range(0));
		std::ranges::iota(std::views::reverse(data), 0);

		run_merge_sort(state, data);
	}

	BENCHMARK(merge_sort_random)
		->RangeMultiplier(2)
		->Range(128, 65536)
		->Complexity();

	BENCHMARK(merge_sort_sorted)
		->RangeMultiplier(2)
		->Range(128, 65536)
		->Complexity();

	BENCHMARK(merge_sort_reverse_sorted)
		->RangeMultiplier(2)
		->Range(128, 65536)
		->Complexity();

} // namespace
