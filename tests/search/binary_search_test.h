#pragma once

#include "ist_internal/search.h"
#include "search.h"
#include "parray.hpp"
#include <utility>
#include <gtest/gtest.h>
#include <cstdint>
#include <iostream>
#include <unordered_set>

TEST(binary_search, single_key)
{
    pasl::pctl::parray<std::pair<int32_t, bool>> keys = {{1, true}};

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
    pasl::pctl::parray<std::pair<int32_t, bool>> keys = {
        {1, true}, {4, true}, {7, true}, {9, true}, {10, true}, {11, true}, {15, true}
    };

    auto [idx, found] = binary_search(keys, -100);
    ASSERT_EQ(0, idx);
    ASSERT_FALSE(found);

    for (uint64_t i = 0; i < keys.size(); ++i)
    {
        auto [cur_idx, cur_found] = binary_search(keys, keys[i].first);
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
    uint32_t max_size = 100'000;
    uint32_t tests_count = 100;
    uint32_t requests_count = 100;

    std::default_random_engine generator(time(nullptr));
    std::uniform_int_distribution<uint32_t> size_distribution(1, max_size);
    std::uniform_int_distribution<int32_t> elements_distribution(-1000000, 1000000);

    for (uint32_t i = 0; i < tests_count; ++i)
    {
        uint32_t cur_size = size_distribution(generator);

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
        pasl::pctl::parray<std::pair<int32_t, bool>> keys(
            keys_v.size(),
            [&keys_v](long i) -> std::pair<int32_t, bool>
            {
                return {keys_v[i], true};
            }
        );

        for (uint32_t j = 0; j < requests_count; ++j)
        {
            int32_t cur_req =  elements_distribution(generator);
            auto [idx, found] = binary_search(keys, cur_req);
            ASSERT_EQ(keys_set.find(cur_req) != keys_set.end(), found);
            if (found)
            {
                ASSERT_EQ(cur_req, keys[idx].first);
            }
            else
            {
                if (idx == keys.size())
                {
                    ASSERT_TRUE(cur_req > keys[keys.size() - 1].first);
                }
                else if (idx == 0)
                {
                    ASSERT_TRUE(cur_req < keys[0].first);
                }
                else
                {
                    ASSERT_TRUE(keys[idx - 1].first < cur_req && cur_req < keys[idx].first);
                }
            }
        }
    }
}