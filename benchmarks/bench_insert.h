#pragma once

#include <benchmark/benchmark.h>
#include <random>
#include "ist_internal/build.h"
#include "ist_internal/node.h"
#include "ist_internal/tree.h"
#include <cstdint>
#include "utils.h"
#include <cassert>
#include <iostream>

static void bench_insert_par(benchmark::State& state) 
{
    assert(false);
    
    uint64_t tree_size = state.range(0);
    uint64_t batch_size = state.range(1);
    int32_t keys_from = state.range(2);
    int32_t keys_to = state.range(3);
    uint64_t size_threshold = 3;

    std::default_random_engine generator(time(nullptr));
    std::uniform_int_distribution<int32_t> elements_distribution(keys_from, keys_to);

    for (auto _ : state) 
    {
        //state.PauseTiming();
        pasl::pctl::parray<int32_t> keys = get_batch(tree_size, generator, elements_distribution);
        pasl::pctl::parray<int32_t> batch = get_batch(batch_size, generator, elements_distribution);
        auto tree = ist_internal(keys, size_threshold);
        //state.ResumeTiming();

        auto start = std::chrono::high_resolution_clock::now();

        pasl::pctl::parray<bool> result = tree.insert(batch);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();

        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
        state.SetIterationTime(elapsed_seconds.count());
    }
}

BENCHMARK(bench_insert_par)
    ->Args({100'000'000, 10'000'000, -100'000'000, 100'000'000})
    ->Unit(benchmark::kMillisecond)
    ->Repetitions(1)
    ->Iterations(5)
    ->UseManualTime();

static void bench_multi_insert_par(benchmark::State& state) 
{
    assert(false);
    
    uint64_t tree_size = state.range(0);
    uint64_t batch_count = state.range(1);
    uint64_t batch_size = state.range(2);
    int32_t keys_from = state.range(3);
    int32_t keys_to = state.range(4);
    uint64_t size_threshold = 3;

    std::default_random_engine generator(time(nullptr));
    std::uniform_int_distribution<int32_t> elements_distribution(keys_from, keys_to);

    for (auto _ : state) 
    {
        std::cout << "Iteration" << std::endl;
        pasl::pctl::parray<int32_t> keys = get_batch(tree_size, generator, elements_distribution);
        auto tree = ist_internal(keys, size_threshold);

        double total_time = 0.0;
        for (uint64_t i = 0; i < batch_count; ++i)
        {
            pasl::pctl::parray<int32_t> batch = get_batch(batch_size, generator, elements_distribution);
            auto start = std::chrono::high_resolution_clock::now();

            pasl::pctl::parray<bool> result = tree.insert(batch);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();

            auto end = std::chrono::high_resolution_clock::now();
            auto elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
            total_time += elapsed_seconds.count();

            pasl::pctl::parray<bool> del_result = tree.remove(batch);
            benchmark::DoNotOptimize(del_result);
            benchmark::ClobberMemory();
        }
        
        state.SetIterationTime(total_time);
    }
}

BENCHMARK(bench_multi_insert_par)
    ->Args({100'000'000, 1'000, 10'000, -100'000'000, 100'000'000})
    ->Unit(benchmark::kMillisecond)
    ->Repetitions(1)
    ->Iterations(5)
    ->UseManualTime();

static void bench_insert_seq(benchmark::State& state) 
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
            auto result = tree.insert(cur_elem);
            benchmark::DoNotOptimize(result);
            // TODO: maybe, ClobberMemory() only befores measuring the end?
            benchmark::ClobberMemory();
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
        state.SetIterationTime(elapsed_seconds.count());
    }
}


BENCHMARK(bench_insert_seq)
    ->Args({100'000'000, 10'000'000, -100'000'000, 100'000'000})
    ->Unit(benchmark::kMillisecond)
    ->Repetitions(1)
    ->Iterations(5)
    ->UseManualTime();