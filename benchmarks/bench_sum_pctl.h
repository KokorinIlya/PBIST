#include <benchmark/benchmark.h>
#include <random>
#include "datapar.hpp"
#include "parray.hpp"
#include "sum.h"
#include <cstdint>

static void bench_sum_pctl(benchmark::State& state) 
{
    std::default_random_engine generator(time(nullptr));
    std::uniform_int_distribution<int32_t> elements_distribution(-1'000'000, 1'000'000);
    uint32_t SIZE = 10'000'000;

    for (auto _ : state) 
    {
        state.PauseTiming();
        pasl::pctl::parray<int32_t> arr(
            SIZE, 
            [&elements_distribution, &generator](uint64_t) 
            { 
                return elements_distribution(generator);
            }
        );
        state.ResumeTiming();

        calc_sum_parallel(arr);
    }
}

BENCHMARK(bench_sum_pctl)
    ->Unit(benchmark::kMillisecond)
    ->Repetitions(5);