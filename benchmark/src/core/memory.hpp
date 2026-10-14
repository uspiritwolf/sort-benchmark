#pragma once

#include <benchmark/benchmark.h>

#include <cstdint>

auto get_default_mem_mgr() -> benchmark::MemoryManager*;

auto get_num_allocs() -> int64_t;

auto get_max_bytes_used() -> int64_t;

auto mem_begin_measuring_section() -> void;

auto mem_end_measuring_section() -> void;

auto fill_memory_usage(benchmark::State& state, benchmark::Counter::Flags flags) -> void;
