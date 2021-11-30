#pragma once

#include "search.h"
#include "parray.hpp"
#include <utility>
#include <gtest/gtest.h>
#include <cstdint>
#include <iostream>

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