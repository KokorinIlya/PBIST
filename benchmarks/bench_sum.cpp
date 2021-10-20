#include "sum.h"
#include <chrono>
#include <random>
#include <iostream>
#include <functional>

int32_t calc_sum_seq(pasl::pctl::parray<int32_t> const& arr)
{
    int32_t res = 0;
    for (uint32_t i = 0; i < arr.size(); ++i)
    {
        res += arr[i];
    }
    return res;
}

uint64_t measure(std::default_random_engine& generator, std::uniform_int_distribution<int32_t>& elements_distribution,
                std::function<int32_t(pasl::pctl::parray<int32_t> const&)> sum_func, uint32_t sz, uint32_t reps)
{
    uint64_t sum = 0;
    for (uint32_t i = 0; i < reps; ++i)
    {
        pasl::pctl::parray<int32_t> arr(
            sz, 
            [&elements_distribution, &generator](long) 
            { 
                return elements_distribution(generator);
            }
        );
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        sum_func(arr);
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        sum += std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
    }
    return sum / reps;
}

int main()
{
    std::default_random_engine generator(time(nullptr));
    std::uniform_int_distribution<int32_t> elements_distribution(-1000000, 1000000);
    uint32_t sz = 10000000;
    uint32_t reps = 10;

    uint64_t res_seq = measure(generator, elements_distribution, &calc_sum_seq, sz, reps);
    std::cout << "Sequential, elapsed " << res_seq << " milliseconds" << std::endl;

    uint64_t res_par = measure(generator, elements_distribution, &calc_sum_parallel, sz, reps);
    std::cout << "Sequential, elapsed " << res_par << " milliseconds" << std::endl;

    return 0;
}