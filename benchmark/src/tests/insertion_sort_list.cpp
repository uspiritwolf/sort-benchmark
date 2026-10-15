#include <core/memory.hpp>

#include <benchmark/benchmark.h>

#include <algorithm>
#include <cassert>
#include <iterator>
#include <list>
#include <random>

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

	void insertion_sort_list(benchmark::State& state)
	{
		std::mt19937 rng(std::random_device{}());

		std::list<int> origin_data(state.range(0));
		std::ranges::generate(origin_data, rng);

		for (auto _ : state)
		{
			state.PauseTiming();
			std::list<int> data_copy(origin_data);
			state.ResumeTiming();

			mem_begin_measuring_section();

			insertion_sort(data_copy);
			assert(std::ranges::is_sorted(data_copy));

			mem_end_measuring_section();
		}

		fill_memory_usage(state, benchmark::Counter::kAvgIterations);
	}

	BENCHMARK(insertion_sort_list)
		->RangeMultiplier(2)
		->Range(1024, 1024 << 6);

} // namespace
