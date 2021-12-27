#pragma once

#include <benchmark/benchmark.h>
#include <random>
#include "datapar.hpp"
#include "parray.hpp"
#include "sum.h"
#include <cstdint>

static void bench_sum_pctl(benchmark::State& state) 
{
    assert(false);
    
    uint64_t size = state.range(0);
    int32_t keys_from = state.range(1);
    int32_t keys_to = state.range(2);

    std::default_random_engine generator(time(nullptr));
    std::uniform_int_distribution<int32_t> elements_distribution(keys_from, keys_to);

    for (auto _ : state) 
    {
        state.PauseTiming();
        pasl::pctl::parray<int32_t> arr(
            size, 
            [&elements_distribution, &generator](uint64_t) 
            { 
                return elements_distribution(generator);
            }
        );
        state.ResumeTiming();

        int32_t sum = calc_sum_parallel(arr);
        benchmark::DoNotOptimize(sum);
        benchmark::ClobberMemory();
    }
}

BENCHMARK(bench_sum_pctl)
    ->Args({10'000'000, -1'000'000, 1'000'000})
    ->Unit(benchmark::kMillisecond)
    ->Repetitions(5)
    ->UseRealTime();