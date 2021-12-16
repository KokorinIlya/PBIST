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

TEST(contains_single, empty)
{
    pasl::pctl::parray<int32_t> keys = {};
    ist_internal<int32_t> tree(keys, 3);
    for (int32_t i = -100; i <= 100; ++i)
    {
        ASSERT_FALSE(tree.contains(i));
    }
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

        auto [tree_keys_set, tree_keys] = get_build_batch<int32_t>(cur_size, generator, elements_distribution);
        ist_internal<int32_t> tree(tree_keys, cur_size_threshold);

        for (uint32_t j = 0; j < REQUESTS_PER_TEST; ++j)
        {
            int32_t cur_req =  elements_distribution(generator);
            bool found = tree.contains(cur_req);
            ASSERT_EQ(tree_keys_set.find(cur_req) != tree_keys_set.end(), found);
        }
    }
}