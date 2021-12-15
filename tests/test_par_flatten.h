#pragma once

#include <gtest/gtest.h>
#include "ist_internal/node.h"
#include "ist_internal/build.h"
#include "config.h"
#include <memory>
#include <cstdint>
#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <random>
#include <algorithm>
#include <unordered_set>

TEST(parallel_flatten, simple)
{
    pasl::pctl::parray<int32_t> keys(
        10,
        [](long i)
        {
            return static_cast<int32_t>(i);
        }
    );
    std::unique_ptr<ist_internal_node<int32_t>> result = build_from_keys(keys, 3);
    auto par_flattened_keys = result->get_keys();
    pasl::pctl::parray<int32_t> exp_par_flattened_keys = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    ASSERT_EQ(exp_par_flattened_keys.size(), par_flattened_keys.size());
    for (uint64_t i = 0; i < par_flattened_keys.size(); ++i)
    {
        ASSERT_EQ(exp_par_flattened_keys[i], par_flattened_keys[i]);
    }
}

TEST(parallel_flatten, stress) 
{
    uint32_t max_size = 100'000;

    std::default_random_engine generator(time(nullptr));
    std::uniform_int_distribution<uint32_t> size_distribution(1, max_size);
    std::uniform_int_distribution<uint32_t> size_threshold_distribution(3, 10);
    std::uniform_int_distribution<int32_t> elements_distribution(-1'000'000, 1'000'000);

    for (uint32_t i = 0; i < TESTS_COUNT; ++i)
    {
        uint32_t cur_size = size_distribution(generator);
        uint32_t cur_size_threshold = size_threshold_distribution(generator);

        std::vector<int32_t> keys_v;
        std::unordered_set<int32_t> keys_set;
        while (keys_v.size() < cur_size)
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
        pasl::pctl::parray<int32_t> keys(
            keys_v.size(),
            [&keys_v](long i)
            {
                return keys_v[i];
            }
        );

        std::unique_ptr<ist_internal_node<int32_t>> result = build_from_keys(keys, cur_size_threshold);
        auto flattened_keys = result->get_keys();
        ASSERT_EQ(keys_v.size(), flattened_keys.size());
        for (uint64_t i = 0; i < flattened_keys.size(); ++i)
        {
            ASSERT_EQ(keys_v[i], flattened_keys[i]);
        }
    }
}