#pragma once

#include <gtest/gtest.h>
#include "ist_internal/node.h"
#include "ist_internal/build.h"
#include <memory>
#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <random>
#include <algorithm>
#include <unordered_set>
#include "test_utils.h"

TEST(tree_building, simple)
{
    pasl::pctl::parray<int32_t> keys(
        10,
        [](long i)
        {
            return static_cast<int32_t>(i);
        }
    );
    ist_internal_node<int32_t>* result = build_from_keys(keys, 3);

    auto stored_keys = result->dump_keys_by_level_seq();

    ASSERT_EQ(stored_keys.size(), 2);

    ASSERT_EQ(stored_keys[0].size(), 1);

    ASSERT_FALSE(stored_keys[0][0].second);
    std::vector<std::pair<int32_t, bool>> exp_top = {{3, true}, {7, true}};
    ASSERT_EQ(stored_keys[0][0].first, exp_top);

    ASSERT_EQ(stored_keys[1].size(), 3);

    ASSERT_TRUE(stored_keys[1][0].second);
    std::vector<std::pair<int32_t, bool>> exp_1 = {{0, true}, {1, true}, {2, true}};
    ASSERT_EQ(stored_keys[1][0].first, exp_1);

    ASSERT_TRUE(stored_keys[1][1].second);
    std::vector<std::pair<int32_t, bool>> exp_2 = {{4, true}, {5, true}, {6, true}};
    ASSERT_EQ(stored_keys[1][1].first, exp_2);

    ASSERT_TRUE(stored_keys[1][2].second);
    std::vector<std::pair<int32_t, bool>> exp_3 = {{8, true}, {9, true}};
    ASSERT_EQ(stored_keys[1][2].first, exp_3);

    auto seq_flattened_keys = result->dump_keys_seq();
    std::vector<int32_t> exp_flattened_keys = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    ASSERT_EQ(exp_flattened_keys, seq_flattened_keys);

    delete result;
}

TEST(tree_building, last_child_empty)
{
    pasl::pctl::parray<int32_t> keys = {0, 1, 2, 3, 4, 5};
    ist_internal_node<int32_t>* result = build_from_keys(keys, 3);

    auto stored_keys = result->dump_keys_by_level_seq();

    ASSERT_EQ(stored_keys.size(), 2);

    ASSERT_EQ(stored_keys[0].size(), 1);

    ASSERT_FALSE(stored_keys[0][0].second);
    std::vector<std::pair<int32_t, bool>> exp_top = {{2, true}, {5, true}};
    ASSERT_EQ(stored_keys[0][0].first, exp_top);

    ASSERT_EQ(stored_keys[1].size(), 3);

    ASSERT_TRUE(stored_keys[1][0].second);
    std::vector<std::pair<int32_t, bool>> exp_1 = {{0, true}, {1, true}};
    ASSERT_EQ(stored_keys[1][0].first, exp_1);

    ASSERT_TRUE(stored_keys[1][1].second);
    std::vector<std::pair<int32_t, bool>> exp_2 = {{3, true}, {4, true}};
    ASSERT_EQ(stored_keys[1][1].first, exp_2);

    ASSERT_TRUE(stored_keys[1][2].second);
    std::vector<std::pair<int32_t, bool>> exp_3;
    ASSERT_EQ(stored_keys[1][2].first, exp_3);

    auto flattened_keys = result->dump_keys_seq();
    ASSERT_EQ(keys.size(), flattened_keys.size());
    for (uint32_t i = 0; i < keys.size(); ++i)
    {
        ASSERT_EQ(keys[i], flattened_keys[i]);
    }

    delete result;
}

TEST(tree_building, empty)
{
    pasl::pctl::parray<int32_t> keys;
    ist_internal_node<int32_t>* result = build_from_keys(keys, 3);
    ASSERT_EQ(nullptr, result);

    delete result;
}

TEST(tree_building, signle_level)
{
    pasl::pctl::parray<int32_t> keys = {0, 1, 2, 3, 4, 5};
    ist_internal_node<int32_t>* result = build_from_keys(keys, 10);

    auto stored_keys = result->dump_keys_by_level_seq();

    ASSERT_EQ(stored_keys.size(), 1);
    ASSERT_EQ(stored_keys[0].size(), 1);
    ASSERT_TRUE(stored_keys[0][0].second);
    std::vector<std::pair<int32_t, bool>> exp_top = {{0, true}, {1, true}, {2, true}, {3, true}, {4, true}, {5, true}};
    ASSERT_EQ(stored_keys[0][0].first, exp_top);

    auto flattened_keys = result->dump_keys_seq();
    ASSERT_EQ(keys.size(), flattened_keys.size());
    for (uint32_t i = 0; i < keys.size(); ++i)
    {
        ASSERT_EQ(keys[i], flattened_keys[i]);
    }

    delete result;
}

TEST(tree_building, stress) 
{
    uint32_t MAX_SIZE = 100'000;
    uint32_t TESTS_COUNT = 200;
    int32_t KEYS_FROM = -1'000'000;
    int32_t KEYS_TO = 1'000'000;

    std::default_random_engine generator(time(nullptr));
    std::uniform_int_distribution<uint32_t> size_distribution(1, MAX_SIZE);
    std::uniform_int_distribution<uint32_t> size_threshold_distribution(3, 10);
    std::uniform_int_distribution<int32_t> elements_distribution(KEYS_FROM, KEYS_TO);

    for (uint32_t i = 0; i < TESTS_COUNT; ++i)
    {
        uint32_t cur_size = size_distribution(generator);
        uint32_t cur_size_threshold = size_threshold_distribution(generator);

        auto tree_keys = get_build_batch<int32_t>(cur_size, generator, elements_distribution).second;

        ist_internal_node<int32_t>* result = build_from_keys(tree_keys, cur_size_threshold);
        auto flattened_keys = result->dump_keys_seq();
        ASSERT_EQ(tree_keys.size(), flattened_keys.size());
        for (uint64_t i = 0; i < tree_keys.size(); ++i)
        {
            ASSERT_EQ(tree_keys[i], flattened_keys[i]);
        }

        if (result == nullptr)
        {
            ASSERT_EQ(0, cur_size);
        }
        else
        {
            ASSERT_EQ(cur_size, result->calc_node_size());
        }

        delete result;
    }
}
