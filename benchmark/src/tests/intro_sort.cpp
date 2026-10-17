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
	constexpr int insertion_sort_threshold = 16;

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

	template <typename Iter>
	struct Partition
	{
		Iter lower;
		Iter upper;
		std::iter_difference_t<Iter> lower_size;
		std::iter_difference_t<Iter> upper_size;
	};

	template <typename Iter>
	auto qs_partition_range(Iter first, Iter last, std::iter_difference_t<Iter> size) -> Partition<Iter>
	{
		const auto pivot = *std::next(first, size / 2);
		Iter lower = first;
		Iter current = first;
		Iter upper = last;
		std::iter_difference_t<Iter> lower_size = 0;
		std::iter_difference_t<Iter> upper_size = 0;

		while (current != upper)
		{
			if (*current < pivot)
			{
				std::iter_swap(lower, current);
				++lower;
				++current;
				++lower_size;
			}
			else if (pivot < *current)
			{
				--upper;
				std::iter_swap(current, upper);
				++upper_size;
			}
			else
			{
				++current;
			}
		}

		return { lower, upper, lower_size, upper_size };
	}

	template <typename Iter>
	void intro_sort_impl(Iter first, Iter last, std::iter_difference_t<Iter> size, int depth)
	{
		struct PendingPartition
		{
			Iter first;
			Iter last;
			std::iter_difference_t<Iter> size;
			int depth;
		};

		std::vector<PendingPartition> pending;
		for (;;)
		{
			while (size > insertion_sort_threshold)
			{
				if (depth == 0)
				{
					heap_sort(first, last);
					size = 0;
					break;
				}
				--depth;
				const auto partition = qs_partition_range(first, last, size);
				if (partition.lower_size < partition.upper_size)
				{
					if (partition.upper_size > 1)
					{
						pending.push_back({ partition.upper, last, partition.upper_size, depth });
					}
					last = partition.lower;
					size = partition.lower_size;
				}
				else
				{
					if (partition.lower_size > 1)
					{
						pending.push_back({ first, partition.lower, partition.lower_size, depth });
					}
					first = partition.upper;
					size = partition.upper_size;
				}
			}

			if (size > 1)
			{
				insertion_sort(first, last);
			}
			if (pending.empty())
			{
				break;
			}

			const auto partition = pending.back();
			pending.pop_back();
			first = partition.first;
			last = partition.last;
			size = partition.size;
			depth = partition.depth;
		}
	}

	template <typename Iter>
	void intro_sort(Iter first, Iter last)
	{
		const auto size = std::distance(first, last);
		int depth = 0;
		for (auto remaining = size; remaining > 1; remaining /= 2)
		{
			depth += 2;
		}
		intro_sort_impl(first, last, size, depth);
	}

	void run_intro_sort(benchmark::State& state, const std::vector<int>& data)
	{
		for (auto _ : state)
		{
			state.PauseTiming();
			std::vector<int> data_copy(data);
			state.ResumeTiming();

			intro_sort(data_copy.begin(), data_copy.end());
			assert(std::ranges::is_sorted(data_copy));
		}

		state.SetComplexityN(state.range(0));
	}

	void intro_sort_random(benchmark::State& state)
	{
		std::mt19937 rng(std::random_device{}());

		std::vector<int> data(state.range(0));
		std::ranges::generate(data, rng);

		run_intro_sort(state, data);
	}

	void intro_sort_sorted(benchmark::State& state)
	{
		std::vector<int> data(state.range(0));
		std::ranges::iota(data, 0);

		run_intro_sort(state, data);
	}

	void intro_sort_reverse_sorted(benchmark::State& state)
	{
		std::vector<int> data(state.range(0));
		std::ranges::iota(std::views::reverse(data), 0);

		run_intro_sort(state, data);
	}

	BENCHMARK(intro_sort_random)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6)
		->Complexity();

	BENCHMARK(intro_sort_sorted)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6)
		->Complexity();

	BENCHMARK(intro_sort_reverse_sorted)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6)
		->Complexity();

} // namespace
