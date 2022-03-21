#pragma once

#include "ist_internal/search.h"
#include "search.h"
#include "parray.hpp"
#include <utility>
#include <gtest/gtest.h>
#include <cstdint>
#include <iostream>
#include "../test_utils.h"

TEST(binary_search, single_key)
{
    pasl::pctl::parray<int32_t> keys = {1};

    auto [idx_1, found_1] = binary_search(keys, -100);
    ASSERT_EQ(0, idx_1);
    ASSERT_FALSE(found_1);

    auto [idx_2, found_2] = binary_search(keys, 1);
    ASSERT_EQ(0, idx_2);
    ASSERT_TRUE(found_2);

    auto [idx_3, found_3] = binary_search(keys, 100);
    ASSERT_EQ(1, idx_3);
    ASSERT_FALSE(found_3);
}

TEST(binary_search, multiple_keys)
{
    pasl::pctl::parray<int32_t> keys = {1, 4, 7, 9, 10, 11, 15};

    auto [idx, found] = binary_search(keys, -100);
    ASSERT_EQ(0, idx);
    ASSERT_FALSE(found);

    for (uint64_t i = 0; i < keys.size(); ++i)
    {
        auto [cur_idx, cur_found] = binary_search(keys, keys[i]);
        ASSERT_EQ(i, cur_idx);
        ASSERT_TRUE(cur_found);
    }

    auto res = binary_search(keys, 100);
    ASSERT_EQ(7, res.first);
    ASSERT_FALSE(res.second);

    res = binary_search(keys, 2);
    ASSERT_EQ(1, res.first);
    ASSERT_FALSE(res.second);

    res = binary_search(keys, 5);
    ASSERT_EQ(2, res.first);
    ASSERT_FALSE(res.second);

    res = binary_search(keys, 8);
    ASSERT_EQ(3, res.first);
    ASSERT_FALSE(res.second);

    res = binary_search(keys, 12);
    ASSERT_EQ(6, res.first);
    ASSERT_FALSE(res.second);
}

TEST(binary_search, stress) 
{
    uint32_t MAX_SIZE = 100'000;
    uint32_t TESTS_COUNT = 200;
    uint32_t REQUESTS_PER_TEST = 200;
    int32_t KEYS_FROM = -1'000'000;
    int32_t KEYS_TO = 1'000'000;

    std::default_random_engine generator(time(nullptr));
    std::uniform_int_distribution<uint32_t> size_distribution(1, MAX_SIZE);
    std::uniform_int_distribution<int32_t> elements_distribution(KEYS_FROM, KEYS_TO);

    for (uint32_t i = 0; i < TESTS_COUNT; ++i)
    {
        uint32_t cur_size = size_distribution(generator);
        auto [keys_set, keys] = get_batch(cur_size, generator, elements_distribution);

        for (uint32_t j = 0; j < REQUESTS_PER_TEST; ++j)
        {
            int32_t cur_req =  elements_distribution(generator);
            auto [idx, found] = binary_search(keys, cur_req);
            ASSERT_EQ(keys_set.find(cur_req) != keys_set.end(), found);
            if (found)
            {
                ASSERT_EQ(cur_req, keys[idx]);
            }
            else
            {
                if (idx == keys.size())
                {
                    ASSERT_TRUE(cur_req > keys[keys.size() - 1]);
                }
                else if (idx == 0)
                {
                    ASSERT_TRUE(cur_req < keys[0]);
                }
                else
                {
                    ASSERT_TRUE(keys[idx - 1] < cur_req && cur_req < keys[idx]);
                }
            }
        }
    }
}