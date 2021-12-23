#pragma once

#include <gtest/gtest.h>
#include "ist_internal/tree.h"
#include <random>
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

void do_test_contains_single_stress(
    uint32_t MAX_TREE_SIZE, uint32_t TESTS_COUNT, uint32_t REQUESTS_PER_TEST,
    int32_t KEYS_FROM, int32_t KEYS_TO, bool CHECK_SIZES)
{
    std::default_random_engine generator(time(nullptr));
    std::uniform_int_distribution<uint32_t> size_distribution(1, MAX_TREE_SIZE);
    std::uniform_int_distribution<int32_t> elements_distribution(KEYS_FROM, KEYS_TO);
    std::uniform_int_distribution<uint32_t> size_threshold_distribution(3, 10);

    for (uint32_t i = 0; i < TESTS_COUNT; ++i)
    {
        uint32_t cur_size = size_distribution(generator);
        uint32_t cur_size_threshold = size_threshold_distribution(generator);

        auto [tree_keys_set, tree_keys] = get_build_batch<int32_t>(cur_size, generator, elements_distribution);
        ist_internal<int32_t> tree(tree_keys, cur_size_threshold);
        if (CHECK_SIZES)
        {
            tree.calc_tree_size();
        }

        for (uint32_t j = 0; j < REQUESTS_PER_TEST; ++j)
        {
            int32_t cur_req =  elements_distribution(generator);
            bool found = tree.contains(cur_req);
            ASSERT_EQ(tree_keys_set.find(cur_req) != tree_keys_set.end(), found);
        }

        if (CHECK_SIZES)
        {
            tree.calc_tree_size();
        }
    }
}

TEST(contains_single, stress)
{
    uint32_t MAX_TREE_SIZE = 100'000;
    uint32_t TESTS_COUNT = 200;
    uint32_t REQUESTS_PER_TEST = 200;
    int32_t KEYS_FROM = -100'000;
    int32_t KEYS_TO = 100'000;

    do_test_contains_single_stress(MAX_TREE_SIZE, TESTS_COUNT, REQUESTS_PER_TEST, KEYS_FROM, KEYS_TO, false);
}

TEST(contains_single, stress_check_sizes)
{
    uint32_t MAX_TREE_SIZE = 10'000;
    uint32_t TESTS_COUNT = 50;
    uint32_t REQUESTS_PER_TEST = 5;
    int32_t KEYS_FROM = -10'000;
    int32_t KEYS_TO = 10'000;

    do_test_contains_single_stress(MAX_TREE_SIZE, TESTS_COUNT, REQUESTS_PER_TEST, KEYS_FROM, KEYS_TO, true);
}