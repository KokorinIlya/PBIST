#pragma once

#include <benchmark/benchmark.h>
#include <random>
#include "ist_internal/build.h"
#include "ist_internal/node.h"
#include <cstdint>
#include "utils.h"

static void bench_tree_build(benchmark::State& state) 
{
    //assert(false);
    
    uint64_t size = state.range(0);
    int32_t keys_from = state.range(1);
    int32_t keys_to = state.range(2);
    uint64_t size_threshold = 3;

    std::default_random_engine generator(time(nullptr));
    std::uniform_int_distribution<int32_t> elements_distribution(keys_from, keys_to);

    for (auto _ : state) 
    {
        state.PauseTiming();
        pasl::pctl::parray<int32_t> keys = get_batch(size, generator, elements_distribution);
        state.ResumeTiming();

        auto result = build_from_keys(keys, size_threshold);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

BENCHMARK(bench_tree_build)
    ->Args({10'000'000, -1'000'000'000, 1'000'000'000})
    ->Unit(benchmark::kMillisecond)
    ->Repetitions(5)
    ->UseRealTime();