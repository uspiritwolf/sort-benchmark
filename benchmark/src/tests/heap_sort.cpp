#include <benchmark/benchmark.h>

#include <algorithm>
#include <cassert>
#include <iterator>
#include <numeric>
#include <random>
#include <ranges>
#include <vector>

namespace
{
	template <typename Iter>
	void sift_down(Iter first, std::iter_difference_t<Iter> root, std::iter_difference_t<Iter> size)
	{
		Iter parent = std::next(first, root);
		while (root < size / 2)
		{
			auto child = (root * 2) + 1;
			Iter largest = std::next(first, child);
			if (child + 1 < size)
			{
				Iter right = std::next(largest);
				if (*largest < *right)
				{
					++child;
					largest = right;
				}
			}

			if (!(*parent < *largest))
			{
				break;
			}

			std::iter_swap(parent, largest);
			root = child;
			parent = largest;
		}
	}

	template <typename Iter>
	void heap_sort(Iter first, Iter last)
	{
		const auto size = std::distance(first, last);
		if (size < 2)
		{
			return;
		}

		for (auto root = size / 2; root > 0;)
		{
			sift_down(first, --root, size);
		}

		Iter end = last;
		for (auto remaining = size - 1; remaining > 0; --remaining)
		{
			std::iter_swap(first, --end);
			sift_down(first, 0, remaining);
		}
	}

	void run_heap_sort_vector(benchmark::State& state, const std::vector<int>& data)
	{
		for (auto _ : state)
		{
			state.PauseTiming();
			std::vector<int> data_copy(data);
			state.ResumeTiming();

			heap_sort(data_copy.begin(), data_copy.end());
			assert(std::ranges::is_sorted(data_copy));
		}

		state.SetComplexityN(state.range(0));
	}

	void heap_sort_vector(benchmark::State& state)
	{
		std::mt19937 rng(std::random_device{}());

		std::vector<int> data(state.range(0));
		std::ranges::generate(data, rng);

		run_heap_sort_vector(state, data);
	}

	void heap_sort_vector_sorted(benchmark::State& state)
	{
		std::vector<int> data(state.range(0));
		std::ranges::iota(data, 0);

		run_heap_sort_vector(state, data);
	}

	void heap_sort_vector_reverse_sorted(benchmark::State& state)
	{
		std::vector<int> data(state.range(0));
		std::ranges::iota(std::views::reverse(data), 0);

		run_heap_sort_vector(state, data);
	}

	BENCHMARK(heap_sort_vector)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6)
		->Complexity();

	BENCHMARK(heap_sort_vector_sorted)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6)
		->Complexity();

	BENCHMARK(heap_sort_vector_reverse_sorted)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6)
		->Complexity();

} // namespace
