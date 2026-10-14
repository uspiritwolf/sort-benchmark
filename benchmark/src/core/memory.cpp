#include "memory.hpp"

#include <cstdlib>
#include <new>

#include <malloc.h>

namespace
{
	class MemoryManager : public benchmark::MemoryManager
	{
	public:
		bool is_running = false;
		bool is_measuring_section = false;
		int64_t num_allocs;
		int64_t max_bytes_used;

		void Start() override
		{
			num_allocs = 0;
			max_bytes_used = 0;

			is_running = true;

			malloc_trim(0);
		}

		void Stop(Result& result) override
		{
			result.num_allocs = num_allocs;
			result.max_bytes_used = max_bytes_used;

			is_running = false;

			malloc_trim(0);
		}

		void allocate(size_t bytes)
		{
			if (is_running && is_measuring_section)
			{
				num_allocs++;
				max_bytes_used += static_cast<int64_t>(bytes);
			}
		}
	};

	std::unique_ptr<MemoryManager> g_memory_manager;
} // namespace

auto get_default_mem_mgr() -> benchmark::MemoryManager*
{
	if (!g_memory_manager)
	{
		g_memory_manager = std::make_unique<MemoryManager>();
	}
	return g_memory_manager.get();
}

auto get_num_allocs() -> int64_t
{
	return g_memory_manager ? g_memory_manager->num_allocs : 0;
}

auto get_max_bytes_used() -> int64_t
{
	return g_memory_manager ? g_memory_manager->max_bytes_used : 0;
}

auto mem_begin_measuring_section() -> void
{
	if (g_memory_manager)
	{
		g_memory_manager->is_measuring_section = true;
	}
}

auto mem_end_measuring_section() -> void
{
	if (g_memory_manager)
	{
		g_memory_manager->is_measuring_section = false;
	}
}

auto fill_memory_usage(benchmark::State& state, benchmark::Counter::Flags flags) -> void
{
	state.counters["avg_memory_usage"] = benchmark::Counter(
		static_cast<double>(get_max_bytes_used()),
		flags,
		benchmark::Counter::OneK::kIs1000
	);

	state.counters["avg_allocs"] = benchmark::Counter(
		static_cast<double>(get_num_allocs()),
		flags,
		benchmark::Counter::OneK::kIs1000
	);
}

auto operator new(std::size_t size) -> void*
{
	if (g_memory_manager)
	{
		g_memory_manager->allocate(size);
	}
	return std::realloc(nullptr, size);
}

auto operator new(std::size_t size, std::align_val_t alignment) -> void*
{
	if (g_memory_manager)
	{
		g_memory_manager->allocate(size);
	}
	return std::aligned_alloc(static_cast<std::size_t>(alignment), size);
}

auto operator delete(void* ptr) noexcept -> void
{
	std::free(ptr);
}

auto operator delete(void* ptr, std::align_val_t al) noexcept -> void
{
	std::free(ptr);
}
