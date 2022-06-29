#pragma once

#include <random>
#include <unordered_set>
#include <vector>
#include <utility>
#include <cstdint>
#include "parray.hpp"
#include <set>

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

template <typename T>
pasl::pctl::parray<T> get_batch_with_prob(
    T left_border, T right_border, double p,
    std::default_random_engine& generator) 
{
    assert(left_border < right_border);
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    std::vector<T> keys_v;
    for (int64_t i = left_border; i < right_border; ++i)
    {
        double x = dist(generator);
        if (x < p)
        {
            keys_v.push_back(i);
        }
    }

    pasl::pctl::parray<T> keys(
        keys_v.size(),
        [&keys_v](uint64_t i)
        {
            return keys_v[i];
        }
    );
    return keys;
}

template <typename T>
std::set<T> get_set(
    uint64_t set_size,
    std::default_random_engine& generator, std::uniform_int_distribution<T>& elements_distribution) 
{
    std::set<T> result;
    while (result.size() < set_size)
    {
        int32_t cur_elem =  elements_distribution(generator);
        result.insert(cur_elem);
    }
    assert(result.size() == set_size);
    return result;
}