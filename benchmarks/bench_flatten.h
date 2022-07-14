#pragma once

#include <benchmark/benchmark.h>
#include <random>
#include "ist_internal/build.h"
#include "ist_internal/node.h"
#include <cstdint>
#include "utils.h"
#include <cassert>

static void bench_flatten_exact(benchmark::State& state) 
{
    assert(false);
    
    uint64_t size = state.range(0);
    int32_t keys_from = state.range(1);
    int32_t keys_to = state.range(2);
    uint64_t size_threshold = state.range(3);

    std::default_random_engine generator(time(nullptr));
    std::uniform_int_distribution<int32_t> elements_distribution(keys_from, keys_to);

    for (auto _ : state) 
    {
        //state.PauseTiming();
        pasl::pctl::parray<int32_t> keys = get_batch(size, generator, elements_distribution);
        auto tree = build_from_keys(keys, size_threshold);
        //state.ResumeTiming();

        auto start = std::chrono::high_resolution_clock::now();

        pasl::pctl::parray<int32_t> result = tree->get_keys();
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();

        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
        state.SetIterationTime(elapsed_seconds.count());
    }
}

BENCHMARK(bench_flatten_exact)
    ->Args({10'000'000, -1'000'000'000, 1'000'000'000, 10})
    ->Unit(benchmark::kMillisecond)
    ->Repetitions(1)
    ->Iterations(5)
    ->UseManualTime();