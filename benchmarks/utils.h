#pragma once

#include <random>
#include <unordered_set>
#include <vector>
#include <utility>
#include <cstdint>
#include "parray.hpp"

template <typename T>
pasl::pctl::parray<T> get_batch(
    uint64_t batch_size,
    std::default_random_engine& generator, std::uniform_int_distribution<T>& elements_distribution) 
{
    std::vector<T> keys_v;
    std::unordered_set<T> keys_set;
    while (keys_v.size() < batch_size)
    {
        assert(keys_v.size() == keys_set.size());
        int32_t cur_elem =  elements_distribution(generator);
        if (keys_set.find(cur_elem) != keys_set.end())
        {
            continue;
        }
        auto insert_res = keys_set.insert(cur_elem);
        assert(insert_res.second);
        keys_v.push_back(cur_elem);
    }
    assert(keys_v.size() == keys_set.size());

    std::sort(keys_v.begin(), keys_v.end());
    pasl::pctl::parray<T> keys(
        keys_v.size(),
        [&keys_v](uint64_t i)
        {
            return keys_v[i];
        }
    );
    return keys;
}