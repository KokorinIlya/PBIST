#pragma once

#include <gtest/gtest.h>
#include "ist_internal/tree.h"
#include <random>
#include "config.h"
#include <unordered_set>
#include "utils.h"
#include <unordered_set>
#include "test_utils.h"

TEST(contains_batch, simple)
{
    pasl::pctl::raw raw_marker;
    pasl::pctl::parray<int32_t> keys = {1, 2, 5, 7, 8, 9, 13, 15, 21, 50, 75, 100, 101, 102, 103};
    std::unordered_set<int32_t> keys_set;
    for (uint64_t i = 0; i < keys.size(); ++i)
    {
        bool inserted = keys_set.insert(keys[i]).second;
        ASSERT_TRUE(inserted);
    }

    ist_internal<int32_t> tree(keys, 3);
    pasl::pctl::parray<int32_t> keys_to_check(
        raw_marker, 2000,
        [](uint64_t idx)
        {
            return idx - 1000;
        }
    );
    pasl::pctl::parray<bool> expected_res(
        raw_marker, 2000,
        [&keys_to_check, &keys_set](uint64_t idx)
        {
            return keys_set.find(keys_to_check[idx]) != keys_set.end();
        }
    );

    pasl::pctl::parray<bool> contains_res = tree.contains(keys_to_check);
    ASSERT_EQ(expected_res.size(), contains_res.size());
    for (uint64_t i = 0; i < contains_res.size(); ++i)
    {
        ASSERT_EQ(expected_res[i], contains_res[i]);
    }
}

TEST(contains_batch, empty_tree)
{
    pasl::pctl::raw raw_marker;
    pasl::pctl::parray<int32_t> keys = {};
    ist_internal<int32_t> tree(keys, 3);

    pasl::pctl::parray<int32_t> keys_to_check(
        raw_marker, 2000,
        [](uint64_t idx)
        {
            return idx - 1000;
        }
    );

    pasl::pctl::parray<bool> contains_res = tree.contains(keys_to_check);
    ASSERT_EQ(keys_to_check.size(), contains_res.size());
    for (uint64_t i = 0; i < contains_res.size(); ++i)
    {
        ASSERT_FALSE(contains_res[i]);
    }
}

TEST(contains_batch, empty_req)
{
    pasl::pctl::raw raw_marker;
    pasl::pctl::parray<int32_t> keys = {1, 7, 9, 100, 103, 105};
    ist_internal<int32_t> tree(keys, 3);

    pasl::pctl::parray<int32_t> req = {};
    pasl::pctl::parray<bool> contains_res = tree.contains(req);
    ASSERT_EQ(0, contains_res.size());
}

TEST(contains_batch, stress)
{
    uint32_t max_tree_size = 100'000;
    uint32_t max_req_size = 100'000;

    std::default_random_engine generator(time(nullptr));
    std::uniform_int_distribution<uint32_t> tree_size_distribution(1, max_tree_size);
    std::uniform_int_distribution<uint32_t> req_size_distribution(1, max_req_size);
    std::uniform_int_distribution<int32_t> elements_distribution(-100'000, 100'000);
    std::uniform_int_distribution<uint32_t> size_threshold_distribution(3, 10);

    for (uint32_t i = 0; i < TESTS_COUNT; ++i)
    {
        uint32_t cur_tree_size = tree_size_distribution(generator);
        uint32_t cur_size_threshold = size_threshold_distribution(generator);

        auto [tree_keys_set, tree_keys] = get_build_batch<int32_t>(
            cur_tree_size, generator, elements_distribution
        );

        ist_internal<int32_t> tree(tree_keys, cur_size_threshold);

        uint32_t cur_req_size = req_size_distribution(generator);
        auto [expected_res, req_keys] = get_contains_batch<int32_t>(
            tree_keys_set, cur_req_size, generator, elements_distribution
        );

        pasl::pctl::parray<bool> contains_res = tree.contains(req_keys);
        ASSERT_EQ(expected_res.size(), contains_res.size());
        for (uint64_t i = 0; i < contains_res.size(); ++i)
        {
            ASSERT_EQ(expected_res[i], contains_res[i]);
        }
    }
}