#pragma once

#include <gtest/gtest.h>
#include "ist_internal/tree.h"
#include <random>
#include "config.h"
#include <unordered_set>

TEST(contains_single, simple)
{
    pasl::pctl::parray<int32_t> keys = {1, 2, 5, 7};
    ist_internal<int32_t> tree(keys, 3);
    for (uint32_t i = 0; i < keys.size(); ++i)
    {
        ASSERT_TRUE(tree.contains(keys[i]));
    }
    ASSERT_FALSE(tree.contains(0));
    ASSERT_FALSE(tree.contains(10));
    ASSERT_FALSE(tree.contains(3));
    ASSERT_FALSE(tree.contains(6));
}

TEST(contains_single, stress)
{
    uint32_t max_size = 100'000;

    std::default_random_engine generator(time(nullptr));
    std::uniform_int_distribution<uint32_t> size_distribution(1, max_size);
    std::uniform_int_distribution<int32_t> elements_distribution(-1'000'000, 1'000'000);
    std::uniform_int_distribution<uint32_t> size_threshold_distribution(3, 10);

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
        ist_internal<int32_t> tree(keys, cur_size_threshold);

        for (uint32_t j = 0; j < REQUESTS_PER_TEST; ++j)
        {
            int32_t cur_req =  elements_distribution(generator);
            bool found = tree.contains(cur_req);
            ASSERT_EQ(keys_set.find(cur_req) != keys_set.end(), found);
        }
    }
}