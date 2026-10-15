#include <core/memory.hpp>

#include <benchmark/benchmark.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <limits>
#include <list>
#include <numeric>
#include <random>
#include <ranges>

namespace
{
	void radix_sort(std::list<int>& data)
	{
		if (data.size() < 2)
		{
			return;
		}

		std::array<std::list<int>, 256> buckets;
		constexpr auto sign_bit = 1u << (std::numeric_limits<unsigned int>::digits - 1);
		for (int shift = 0; shift < std::numeric_limits<unsigned int>::digits; shift += 8)
		{
			while (!data.empty())
			{
				auto node = data.begin();
				// Match signed ordering while distributing existing nodes.
				const auto key = static_cast<unsigned int>(*node) ^ sign_bit;
				auto& bucket = buckets[(key >> shift) & 255u];
				bucket.splice(bucket.end(), data, node);
			}
			for (auto& bucket : buckets)
			{
				data.splice(data.end(), bucket);
			}
		}
	}

	void run_radix_sort_list(benchmark::State& state, const std::list<int>& data)
	{
		for (auto _ : state)
		{
			state.PauseTiming();
			std::list<int> data_copy(data);
			state.ResumeTiming();

			mem_begin_measuring_section();

			radix_sort(data_copy);
			assert(std::ranges::is_sorted(data_copy));

			mem_end_measuring_section();
		}

		fill_memory_usage(state, benchmark::Counter::kAvgIterations);
	}

	void radix_sort_list(benchmark::State& state)
	{
		std::mt19937 rng(std::random_device{}());

		std::list<int> data(state.range(0));
		std::ranges::generate(data, rng);

		run_radix_sort_list(state, data);
	}

	void radix_sort_list_sorted(benchmark::State& state)
	{
		std::list<int> data(state.range(0));
		std::ranges::iota(data, 0);

		run_radix_sort_list(state, data);
	}

	void radix_sort_list_reverse_sorted(benchmark::State& state)
	{
		std::list<int> data(state.range(0));
		std::ranges::iota(std::views::reverse(data), 0);

		run_radix_sort_list(state, data);
	}

	BENCHMARK(radix_sort_list)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6);

	BENCHMARK(radix_sort_list_sorted)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6);

	BENCHMARK(radix_sort_list_reverse_sorted)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6);

} // namespace
