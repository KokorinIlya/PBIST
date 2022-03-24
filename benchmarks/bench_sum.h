#pragma once

#include <benchmark/benchmark.h>
#include <random>
#include "datapar.hpp"
#include "parray.hpp"
#include "sum.h"
#include <cstdint>
#include <chrono>
#include <iostream>
#include <cassert>
//#include <hwloc.h>

static void bench_sum_pctl(benchmark::State& state) 
{
    assert(false);
    
    uint64_t size = state.range(0);
    int64_t keys_from = state.range(1);
    int64_t keys_to = state.range(2);

    std::default_random_engine generator(time(nullptr));
    std::uniform_int_distribution<int64_t> elements_distribution(keys_from, keys_to);

    for (auto _ : state) 
    {
        //state.PauseTiming();
        pasl::pctl::parray<int64_t> arr(pasl::pctl::raw{}, size);
        for (uint64_t i = 0; i < size; ++i) 
        {
            arr[i] = elements_distribution(generator);
        }
        //state.ResumeTiming();

        auto start = std::chrono::high_resolution_clock::now();
        
        int64_t sum = calc_sum_parallel(arr);
        benchmark::DoNotOptimize(sum);
        benchmark::ClobberMemory();

        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
        state.SetIterationTime(elapsed_seconds.count());
    }
}

BENCHMARK(bench_sum_pctl)
    ->Args({1'000'000'000, -1'000'000, 1'000'000})
    ->Unit(benchmark::kMillisecond)
    ->Repetitions(1)
    ->Iterations(1)
    ->UseManualTime();