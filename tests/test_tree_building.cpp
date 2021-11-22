#include <gtest/gtest.h>
#include "ist_internal/node.h"
#include "ist_internal/build.h"
#include <memory>
#include <cstdint>
#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <random>
#include <algorithm>
#include <iostream>
#include <utils.h>

TEST(tree_building, simple)
{
    pasl::pctl::parray<int32_t> keys(
        10,
        [](long i)
        {
            return static_cast<int32_t>(i);
        }
    );
    std::unique_ptr<ist_internal_node<int32_t>> result = build_from_keys(keys, 3);

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

    auto par_flattened_keys = result->get_keys();
    pasl::pctl::parray<int32_t> exp_par_flattened_keys = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    ASSERT_EQ(exp_par_flattened_keys.size(), par_flattened_keys.size());
    for (uint64_t i = 0; i < par_flattened_keys.size(); ++i)
    {
        ASSERT_EQ(exp_par_flattened_keys[i], par_flattened_keys[i]);
    }
}

TEST(tree_building, last_child_empty)
{
    pasl::pctl::parray<int32_t> keys = {0, 1, 2, 3, 4, 5};
    std::unique_ptr<ist_internal_node<int32_t>> result = build_from_keys(keys, 3);

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
}

TEST(tree_building, empty)
{
    pasl::pctl::parray<int32_t> keys;
    std::unique_ptr<ist_internal_node<int32_t>> result = build_from_keys(keys, 3);
    ASSERT_EQ(nullptr, result.get());
}

TEST(tree_building, signle_level)
{
    pasl::pctl::parray<int32_t> keys = {0, 1, 2, 3, 4, 5};
    std::unique_ptr<ist_internal_node<int32_t>> result = build_from_keys(keys, 10);

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
}

TEST(tree_building, stress) 
{
    uint32_t max_size = 100'000;
    uint32_t tests_count = 200;

    std::default_random_engine generator(time(nullptr));
    std::uniform_int_distribution<uint32_t> size_distribution(1, max_size);
    std::uniform_int_distribution<uint32_t> size_threshold_distribution(3, 10);
    std::uniform_int_distribution<int32_t> elements_distribution(-1000000, 1000000);

    for (uint32_t i = 0; i < tests_count; ++i)
    {
        uint32_t cur_size = size_distribution(generator);
        uint32_t cur_size_threshold = size_threshold_distribution(generator);

        std::vector<int32_t> keys_v(cur_size);
        for (uint32_t j = 0; j < cur_size; ++j)
        {
            keys_v[j] = elements_distribution(generator);
        }
        std::sort(keys_v.begin(), keys_v.end());
        pasl::pctl::parray<int32_t> keys(
            keys_v.size(),
            [&keys_v](long i)
            {
                return keys_v[i];
            }
        );

        std::unique_ptr<ist_internal_node<int32_t>> result = build_from_keys(keys, cur_size_threshold);
        auto flattened_keys = result->dump_keys_seq();
        ASSERT_EQ(keys_v, flattened_keys);
    }
}
