#include <benchmark/benchmark.h>

#include "midi2/adt/plru_cache.hpp"

template <std::unsigned_integral T, unsigned Sets, unsigned Ways>
static void bm_plru_cache256(benchmark::State& state) {
  midi2::adt::plru_cache<T, std::string, Sets, Ways> cache;
  auto count = T{0};
  for (auto _ : state) {
    auto key = (count / 8) % 1024;
    for (auto ctr = T{0}; ctr < T{8}; ++ctr) {
      cache.access(key + ctr, [count]() { return std::to_string((count + 1e6) / 3.2) + "#"; });
    }
    ++count;
  }
}

// (*) NEON optimized
BENCHMARK(bm_plru_cache256<std::uint16_t, 4, 4>);  // (*)
BENCHMARK(bm_plru_cache256<std::uint16_t, 4, 8>);  // (*)

BENCHMARK(bm_plru_cache256<std::uint32_t, 4, 4>);  // (*)
BENCHMARK(bm_plru_cache256<std::uint32_t, 4, 8>);
BENCHMARK(bm_plru_cache256<std::uint32_t, 4, 16>);
BENCHMARK(bm_plru_cache256<std::uint32_t, 2, 16>);

// 256 cache entries
BENCHMARK(bm_plru_cache256<std::uint32_t, 128, 2>);
BENCHMARK(bm_plru_cache256<std::uint32_t, 64, 4>);  // (*)
BENCHMARK(bm_plru_cache256<std::uint32_t, 32, 8>);
BENCHMARK(bm_plru_cache256<std::uint32_t, 16, 16>);

// 256 cache entries
BENCHMARK(bm_plru_cache256<std::uint16_t, 128, 2>);
BENCHMARK(bm_plru_cache256<std::uint16_t, 64, 4>);  // (*)
BENCHMARK(bm_plru_cache256<std::uint16_t, 32, 8>);  // (*)
BENCHMARK(bm_plru_cache256<std::uint16_t, 16, 16>);

BENCHMARK_MAIN();
