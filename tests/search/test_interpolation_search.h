#pragma once

#include "ist_internal/search.h"
#include "ist_internal/build.h"
#include "search.h"
#include "parray.hpp"
#include <utility>
#include <gtest/gtest.h>
#include <cstdint>
#include <iostream>
#include "utils.h"
#include "../test_utils.h"

TEST(interpolation_search, single_key)
{
    pasl::pctl::parray<int32_t> keys = {1};
    pasl::pctl::parray<uint64_t> id = build_id(keys, 1);

    auto [idx_1, found_1] = interpolation_search(keys, -100, id);
    ASSERT_EQ(0, idx_1);
    ASSERT_FALSE(found_1);

    auto [idx_2, found_2] = interpolation_search(keys, 1, id);
    ASSERT_EQ(0, idx_2);
    ASSERT_TRUE(found_2);

    auto [idx_3, found_3] = interpolation_search(keys, 100, id);
    ASSERT_EQ(1, idx_3);
    ASSERT_FALSE(found_3);
}

TEST(interpolation_search, multiple_keys)
{
    pasl::pctl::parray<int32_t> keys = {1, 4, 7, 9, 10, 11, 15};
    pasl::pctl::parray<uint64_t> id = build_id(keys, keys.size());

    auto [idx, found] = interpolation_search(keys, -100, id);
    ASSERT_EQ(0, idx);
    ASSERT_FALSE(found);

    for (uint64_t i = 0; i < keys.size(); ++i)
    {
        auto [cur_idx, cur_found] = interpolation_search(keys, keys[i], id);
        ASSERT_EQ(i, cur_idx);
        ASSERT_TRUE(cur_found);
    }

    auto res = interpolation_search(keys, 100, id);
    ASSERT_EQ(7, res.first);
    ASSERT_FALSE(res.second);

    res = interpolation_search(keys, 2, id);
    ASSERT_EQ(1, res.first);
    ASSERT_FALSE(res.second);

    res = interpolation_search(keys, 5, id);
    ASSERT_EQ(2, res.first);
    ASSERT_FALSE(res.second);

    res = interpolation_search(keys, 8, id);
    ASSERT_EQ(3, res.first);
    ASSERT_FALSE(res.second);

    res = interpolation_search(keys, 12, id);
    ASSERT_EQ(6, res.first);
    ASSERT_FALSE(res.second);
}

TEST(interpolation_search, stress) 
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
        uint32_t id_size = size_distribution(generator);

        auto [keys_set, keys] = get_batch(cur_size, generator, elements_distribution);
        pasl::pctl::parray<uint64_t> id = build_id(keys, id_size);

        for (uint32_t j = 0; j < REQUESTS_PER_TEST; ++j)
        {
            int32_t cur_req =  elements_distribution(generator);
            auto [idx, found] = interpolation_search(keys, cur_req, id);
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
