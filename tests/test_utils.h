#pragma once

#include <random>
#include <unordered_set>
#include <vector>
#include <utility>
#include <cstdint>
#include "parray.hpp"

template <typename T>
std::pair<std::unordered_set<T>, pasl::pctl::parray<T>> get_build_batch(
    uint64_t batch_size,
    std::default_random_engine& generator, std::uniform_int_distribution<T>& elements_distribution) 
{
    std::vector<T> keys_v; // TODO: common code
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
    return {keys_set, keys};
}

template <typename T>
std::pair<pasl::pctl::parray<bool>, pasl::pctl::parray<T>> get_contains_batch(
    std::unordered_set<T> const& tree_keys, uint64_t batch_size,
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
    pasl::pctl::parray<bool> exp_res(
        keys.size(),
        [&keys, &tree_keys](uint64_t i)
        {
            return tree_keys.find(keys[i]) != tree_keys.end();
        }
    );
    
    return {exp_res, keys};
}

template <typename T>
std::pair<std::vector<bool>, pasl::pctl::parray<T>> get_insert_batch(
    std::unordered_set<T>& tree_keys, uint64_t batch_size,
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

    std::vector<bool> exp_res;
    for (uint64_t i = 0; i < keys.size(); ++i)
    {
        bool inserted = tree_keys.insert(keys[i]).second;
        exp_res.push_back(inserted);
    }
    
    return {exp_res, keys};
}

template <typename T>
std::pair<std::vector<bool>, pasl::pctl::parray<T>> get_remove_batch(
    std::unordered_set<T>& tree_keys, uint64_t batch_size,
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

    std::vector<bool> exp_res;
    for (uint64_t i = 0; i < keys.size(); ++i)
    {
        auto it = tree_keys.find(keys[i]);
        if (it != tree_keys.end())
        {
            exp_res.push_back(true);
            tree_keys.erase(it);
        }
        else
        {
            exp_res.push_back(false);
        }
    }
    
    return {exp_res, keys};
}
