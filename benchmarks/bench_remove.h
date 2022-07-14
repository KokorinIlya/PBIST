#pragma once

#include <benchmark/benchmark.h>
#include <random>
#include "ist_internal/build.h"
#include "ist_internal/node.h"
#include "ist_internal/tree.h"
#include <cstdint>
#include "utils.h"
#include <cassert>

static void bench_remove_par_exact(benchmark::State& state) 
{
    assert(false);
    
    uint64_t tree_size = state.range(0);
    uint64_t batch_size = state.range(1);
    int32_t keys_from = state.range(2);
    int32_t keys_to = state.range(3);
    uint64_t size_threshold = state.range(4);

    std::default_random_engine generator(time(nullptr));
    std::uniform_int_distribution<int32_t> elements_distribution(keys_from, keys_to);

    for (auto _ : state) 
    {
        std::cout << "Iteration" << std::endl;
        //state.PauseTiming();
        pasl::pctl::parray<int32_t> keys = get_batch(tree_size, generator, elements_distribution);
        pasl::pctl::parray<int32_t> batch = get_batch(batch_size, generator, elements_distribution);
        auto tree = ist_internal(keys, size_threshold);
        //state.ResumeTiming();

        auto start = std::chrono::high_resolution_clock::now();

        pasl::pctl::parray<bool> result = tree.remove(batch);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();

        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
        state.SetIterationTime(elapsed_seconds.count());
    }
}

BENCHMARK(bench_remove_par_exact)
    ->Args({100'000'000, 10'000'000, -100'000'000, 100'000'000, 10})
    ->Unit(benchmark::kMillisecond)
    ->Repetitions(10)
    ->Iterations(1)
    ->UseManualTime();

static void bench_remove_par_approx(benchmark::State& state) 
{
    assert(false);
    
    uint64_t tree_size = state.range(0);
    uint32_t batch_size_div = state.range(1);
    uint64_t size_threshold = state.range(2);

    uint64_t batch_size = tree_size / batch_size_div;
    int32_t keys_from = -tree_size;
    int32_t keys_to = tree_size;
    double tree_add_p = 0.5;
    double batch_add_p = tree_add_p / batch_size_div;

    std::default_random_engine generator(time(nullptr));

    for (auto _ : state) 
    {
        //state.PauseTiming();
        pasl::pctl::parray<int32_t> keys = get_batch_with_prob(keys_from, keys_to, tree_add_p, generator);
        pasl::pctl::parray<int32_t> batch = get_batch_with_prob(keys_from, keys_to, batch_add_p, generator);
        auto tree = ist_internal(keys, size_threshold);
        //state.ResumeTiming();

        auto start = std::chrono::high_resolution_clock::now();

        pasl::pctl::parray<bool> result = tree.remove(batch);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();

        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
        state.SetIterationTime(elapsed_seconds.count());
    }
}

BENCHMARK(bench_remove_par_approx)
    ->Args({100'000'000, 10, 10})
    ->Unit(benchmark::kMillisecond)
    ->Repetitions(1)
    ->Iterations(1)
    ->UseManualTime();

static void bench_remove_seq(benchmark::State& state) 
{
    assert(false);
    
    uint64_t tree_size = state.range(0);
    uint64_t batch_size = state.range(1);
    int32_t keys_from = state.range(2);
    int32_t keys_to = state.range(3);

    std::default_random_engine generator(time(nullptr));
    std::uniform_int_distribution<int32_t> elements_distribution(keys_from, keys_to);

    for (auto _ : state) 
    {
        //state.PauseTiming();
        std::set<int32_t> tree = get_set(tree_size, generator, elements_distribution);
        pasl::pctl::parray<int32_t> batch = get_batch(batch_size, generator, elements_distribution);
        //state.ResumeTiming();

        auto start = std::chrono::high_resolution_clock::now();

        for (int32_t cur_elem : batch)
        {
            auto result = tree.erase(cur_elem);
            benchmark::DoNotOptimize(result);
            // TODO: maybe, ClobberMemory() only befores measuring the end?
            benchmark::ClobberMemory();
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
        state.SetIterationTime(elapsed_seconds.count());
    }
}


BENCHMARK(bench_remove_seq)
    ->Args({100'000'000, 10'000'000, -100'000'000, 100'000'000})
    ->Unit(benchmark::kMillisecond)
    ->Repetitions(1)
    ->Iterations(1)
    ->UseManualTime();