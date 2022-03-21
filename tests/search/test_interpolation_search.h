#pragma once

#include "ist_internal/search.h"
#include "ist_internal/build.h"
#include "search.h"
#include "parray.hpp"
#include <utility>
#include <gtest/gtest.h>
#include <cstdint>
#include <iostream>
#include <unordered_set>
#include "utils.h"

TEST(interpolation_search, single_key)
{
    pasl::pctl::parray<std::pair<int32_t, bool>> keys = {{1, true}};
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
    pasl::pctl::parray<std::pair<int32_t, bool>> keys = {
        {1, true}, {4, true}, {7, true}, {9, true}, {10, true}, {11, true}, {15, true}
    };
    pasl::pctl::parray<uint64_t> id = build_id(keys, keys.size());

    auto [idx, found] = interpolation_search(keys, -100, id);
    ASSERT_EQ(0, idx);
    ASSERT_FALSE(found);

    for (uint64_t i = 0; i < keys.size(); ++i)
    {
        auto [cur_idx, cur_found] = interpolation_search(keys, keys[i].first, id);
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

        // TODO: use get_batch here
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

        pasl::pctl::parray<uint64_t> id = build_id(keys, id_size);

        for (uint32_t j = 0; j < REQUESTS_PER_TEST; ++j)
        {
            int32_t cur_req =  elements_distribution(generator);
            auto [idx, found] = interpolation_search(keys, cur_req, id);
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
