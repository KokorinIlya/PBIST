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
#include <hwloc.h>

static void bench_sum_pctl(benchmark::State& state) 
{
    assert(false);

/*-----------------------------------------
    hwloc_topology_t    topology;
    hwloc_topology_init (&topology);
    hwloc_topology_load (topology);
    bool numa_alloc_interleaved = true;//(proc == 1) ? false : true;
//    numa_alloc_interleaved =
//      deepsea::cmdline::parse_or_default_bool("numa_alloc_interleaved", numa_alloc_interleaved, false);
    int cpus = hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_CORE);
    std::cout << "Number of CPUs: " << cpus << "\n";
    if (numa_alloc_interleaved) {
      hwloc_cpuset_t all_cpus =
        hwloc_bitmap_dup (hwloc_topology_get_topology_cpuset (topology));
      int err = hwloc_set_membind(topology, all_cpus, HWLOC_MEMBIND_INTERLEAVE, 0);
      if (err < 0)
        printf("Warning: failed to set NUMA round-robin allocation policy\n");
    }
-----------------------------------------*/
    
    uint64_t size = state.range(0);
    int64_t keys_from = state.range(1);
    int64_t keys_to = state.range(2);

    std::default_random_engine generator(time(nullptr));
    std::uniform_int_distribution<int64_t> elements_distribution(keys_from, keys_to);

    for (auto _ : state) 
    {
        //state.PauseTiming();
        pasl::pctl::parray<int64_t> arr(pasl::pctl::raw{}, size);
        for (int i = 0; i < size; i++) 
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