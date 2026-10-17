#include <benchmark/benchmark.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <limits>
#include <numeric>
#include <random>
#include <ranges>
#include <type_traits>
#include <vector>

namespace
{
	template <typename Iter>
	void radix_sort(Iter first, Iter last)
	{
		static_assert(std::is_same_v<std::iter_value_t<Iter>, int>);
		const auto size = std::distance(first, last);
		if (size < 2)
		{
			return;
		}

		std::vector<int> buffer(size);
		constexpr auto sign_bit = 1u << (std::numeric_limits<unsigned int>::digits - 1);
		for (int shift = 0; shift < std::numeric_limits<unsigned int>::digits; shift += 8)
		{
			std::array<std::size_t, 256> offsets{};
			for (Iter i = first; i != last; ++i)
			{
				// Flipping the sign bit maps signed order to unsigned key order.
				const auto key = static_cast<unsigned int>(*i) ^ sign_bit;
				++offsets[(key >> shift) & 255u];
			}

			std::size_t total = 0;
			for (auto& offset : offsets)
			{
				const auto count = offset;
				offset = total;
				total += count;
			}

			for (Iter i = first; i != last; ++i)
			{
				const auto key = static_cast<unsigned int>(*i) ^ sign_bit;
				buffer[offsets[(key >> shift) & 255u]++] = *i;
			}
			std::copy(buffer.begin(), buffer.end(), first);
		}
	}

	void run_radix_sort(benchmark::State& state, const std::vector<int>& data)
	{
		for (auto _ : state)
		{
			state.PauseTiming();
			std::vector<int> data_copy(data);
			state.ResumeTiming();

			radix_sort(data_copy.begin(), data_copy.end());
			assert(std::ranges::is_sorted(data_copy));
		}

		state.SetComplexityN(state.range(0));
	}

	void radix_sort_random(benchmark::State& state)
	{
		std::mt19937 rng(std::random_device{}());

		std::vector<int> data(state.range(0));
		std::ranges::generate(data, rng);

		run_radix_sort(state, data);
	}

	void radix_sort_sorted(benchmark::State& state)
	{
		std::vector<int> data(state.range(0));
		std::ranges::iota(data, 0);

		run_radix_sort(state, data);
	}

	void radix_sort_reverse_sorted(benchmark::State& state)
	{
		std::vector<int> data(state.range(0));
		std::ranges::iota(std::views::reverse(data), 0);

		run_radix_sort(state, data);
	}

	BENCHMARK(radix_sort_random)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6)
		->Complexity();

	BENCHMARK(radix_sort_sorted)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6)
		->Complexity();

	BENCHMARK(radix_sort_reverse_sorted)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6)
		->Complexity();

} // namespace
