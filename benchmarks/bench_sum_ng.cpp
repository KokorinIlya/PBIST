#include <random>
#include "datapar.hpp"
#include "parray.hpp"
#include "sum.h"
#include <cstdint>
#include <chrono>
#include <iostream>
#include <cassert>
//#include <benchmark/benchmark.h>

int main()
{
    uint64_t size = 1'000'000'000;
    int64_t keys_from = -1'000'000;
    int64_t keys_to = 1'000'000;
    uint32_t repetitions = 5;

    std::default_random_engine generator(time(nullptr));
    std::uniform_int_distribution<int64_t> elements_distribution(keys_from, keys_to);

    double total = 0.0;

    for (uint32_t i = 0; i < repetitions; ++i)
    {
        pasl::pctl::parray<int64_t> arr(pasl::pctl::raw{}, size);
        for (int i = 0; i < size; i++) 
        {
            arr[i] = elements_distribution(generator);
        }

         auto start = std::chrono::high_resolution_clock::now();
        
        int64_t sum = calc_sum_parallel(arr);
        //benchmark::DoNotOptimize(sum);
        //benchmark::ClobberMemory();

        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
        std::cout << "Iteration " << i + 1 << ", elapsed " << elapsed_seconds.count() << " seconds" << std::endl;
        total += elapsed_seconds.count();
    }
    std::cout << "Average: " << total / repetitions << " seconds" << std::endl;
}