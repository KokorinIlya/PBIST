#pragma once

#include <benchmark/benchmark.h>
#include <random>
#include "ist_internal/build.h"
#include "ist_internal/node.h"
#include <cstdint>
#include "utils.h"
#include <cassert>

static void bench_build_exact(benchmark::State& state) 
{
    assert(false);
    
    uint64_t size = state.range(0);
    int32_t keys_from = state.range(1);
    int32_t keys_to = state.range(2);
    uint64_t size_threshold = 3;

    std::default_random_engine generator(time(nullptr));
    std::uniform_int_distribution<int32_t> elements_distribution(keys_from, keys_to);

    for (auto _ : state) 
    {
        //state.PauseTiming();
        pasl::pctl::parray<int32_t> keys = get_batch(size, generator, elements_distribution);
        //state.ResumeTiming();

        auto start = std::chrono::high_resolution_clock::now();

        ist_internal_node<int32_t>* result = build_from_keys(keys, size_threshold);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();

        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
        state.SetIterationTime(elapsed_seconds.count());

        delete result;
    }
}

BENCHMARK(bench_build_exact)
    ->Args({100'000'000, -1'000'000'000, 1'000'000'000})
    ->Unit(benchmark::kMillisecond)
    ->Repetitions(1)
    ->Iterations(5)
    ->UseManualTime();

static void bench_build_approx(benchmark::State& state) 
{
    assert(false);
    
    uint64_t size = state.range(0);
    int32_t keys_from = -size;
    int32_t keys_to = size;
    uint64_t size_threshold = 3;

    std::default_random_engine generator(time(nullptr));

    for (auto _ : state) 
    {
        //state.PauseTiming();
        pasl::pctl::parray<int32_t> keys = get_batch_with_prob(keys_from, keys_to, 0.5, generator);
        //state.ResumeTiming();

        auto start = std::chrono::high_resolution_clock::now();

        ist_internal_node<int32_t>* result = build_from_keys(keys, size_threshold);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();

        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
        state.SetIterationTime(elapsed_seconds.count());

        delete result;
    }
}

BENCHMARK(bench_build_approx)
    ->Args({100'000'000})
    ->Unit(benchmark::kMillisecond)
    ->Repetitions(1)
    ->Iterations(5)
    ->UseManualTime();