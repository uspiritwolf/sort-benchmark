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
	void qs_impl(Iter first, Iter last, std::iter_difference_t<Iter> size)
	{
		struct PendingPartition
		{
			Iter first;
			Iter last;
			std::iter_difference_t<Iter> size;
		};

		std::vector<PendingPartition> pending;
		for (;;)
		{
			while (size > 1)
			{
				const auto partition = qs_partition_range(first, last, size);

				// Process the smaller partition first to bound pending stack size.
				if (partition.lower_size < partition.upper_size)
				{
					if (partition.upper_size > 1)
					{
						pending.push_back({ partition.upper, last, partition.upper_size });
					}
					last = partition.lower;
					size = partition.lower_size;
				}
				else
				{
					if (partition.lower_size > 1)
					{
						pending.push_back({ first, partition.lower, partition.lower_size });
					}
					first = partition.upper;
					size = partition.upper_size;
				}
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
		}
	}

	template <typename Iter>
	void quick_sort(Iter first, Iter last)
	{
		qs_impl(first, last, std::distance(first, last));
	}

	void run_quick_sort(benchmark::State& state, const std::vector<int>& data)
	{
		for (auto _ : state)
		{
			state.PauseTiming();
			std::vector<int> data_copy(data);
			state.ResumeTiming();

			quick_sort(data_copy.begin(), data_copy.end());
			assert(std::ranges::is_sorted(data_copy));
		}

		state.SetComplexityN(state.range(0));
	}

	void quick_sort_random(benchmark::State& state)
	{
		std::mt19937 rng(std::random_device{}());

		std::vector<int> data(state.range(0));
		std::ranges::generate(data, rng);

		run_quick_sort(state, data);
	}

	void quick_sort_sorted(benchmark::State& state)
	{
		std::vector<int> data(state.range(0));
		std::ranges::iota(data, 0);

		run_quick_sort(state, data);
	}

	void quick_sort_reverse_sorted(benchmark::State& state)
	{
		std::vector<int> data(state.range(0));
		std::ranges::iota(std::views::reverse(data), 0);

		run_quick_sort(state, data);
	}

	BENCHMARK(quick_sort_random)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6)
		->Complexity();

	BENCHMARK(quick_sort_sorted)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6)
		->Complexity();

	BENCHMARK(quick_sort_reverse_sorted)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6)
		->Complexity();

} // namespace
