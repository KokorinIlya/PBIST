#pragma once

#include <benchmark/benchmark.h>
#include <random>
#include "ist_internal/search.h"
#include "ist_internal/build.h"
#include <cstdint>
#include "utils.h"
#include <cassert>
#include <vector>

static void bench_binary_search_uniform(benchmark::State& state) 
{
    assert(false);
    
    uint64_t array_size = state.range(0);
    uint32_t req_count = state.range(1);
    int32_t keys_from = state.range(2);
    int32_t keys_to = state.range(3);

    std::default_random_engine generator(time(nullptr));
    std::uniform_int_distribution<int32_t> elements_distribution(keys_from, keys_to);

    for (auto _ : state) 
    {
        //state.PauseTiming();
        pasl::pctl::parray<int32_t> keys = get_batch(array_size, generator, elements_distribution);
        std::vector<int32_t> requests;
        for (uint32_t i = 0; i < req_count; ++i)
        {
            requests.push_back(elements_distribution(generator));
        }
        //state.ResumeTiming();

        auto start = std::chrono::high_resolution_clock::now();

        for (int32_t req : requests)
        {
            auto result = binary_search(keys, req);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
        state.SetIterationTime(elapsed_seconds.count());
    }
}

BENCHMARK(bench_binary_search_uniform)
    ->Args({10'000, 1'000'000, -1'000'000'000, 1'000'000'000})
    ->Unit(benchmark::kMillisecond)
    ->Repetitions(5)
    ->Iterations(10)
    ->UseManualTime();

static void bench_interpolation_search_uniform(benchmark::State& state) 
{
    assert(false);
    
    uint64_t array_size = state.range(0);
    uint32_t req_count = state.range(1);
    int32_t keys_from = state.range(2);
    int32_t keys_to = state.range(3);

    std::default_random_engine generator(time(nullptr));
    std::uniform_int_distribution<int32_t> elements_distribution(keys_from, keys_to);

    for (auto _ : state) 
    {
        //state.PauseTiming();
        pasl::pctl::parray<int32_t> keys = get_batch(array_size, generator, elements_distribution);
        pasl::pctl::parray<uint64_t> id = build_id(keys, array_size);
        std::vector<int32_t> requests;
        for (uint32_t i = 0; i < req_count; ++i)
        {
            requests.push_back(elements_distribution(generator));
        }
        //state.ResumeTiming();

        auto start = std::chrono::high_resolution_clock::now();

        for (int32_t req : requests)
        {
            auto result = interpolation_search(keys, req, id);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
        state.SetIterationTime(elapsed_seconds.count());
    }
}

BENCHMARK(bench_interpolation_search_uniform)
    ->Args({10'000, 1'000'000, -1'000'000'000, 1'000'000'000})
    ->Unit(benchmark::kMillisecond)
    ->Repetitions(5)
    ->Iterations(10)
    ->UseManualTime();