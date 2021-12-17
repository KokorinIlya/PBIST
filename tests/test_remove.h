#pragma once

#include <gtest/gtest.h>
#include "ist_internal/tree.h"
#include <random>
#include "config.h"
#include <unordered_set>
#include "utils.h"
#include <unordered_set>
#include "config.h"

TEST(remove, simple_non_batch_contains)
{
    pasl::pctl::raw raw_marker;
    pasl::pctl::parray<int32_t> keys_construct = {1, 2, 5, 7, 8, 9, 13, 15, 21, 50, 75, 100, 101, 102, 103};

    std::unordered_set<int32_t> keys_set;
    for (uint64_t i = 0; i < keys_construct.size(); ++i)
    {
        bool inserted = keys_set.insert(keys_construct[i]).second;
        ASSERT_TRUE(inserted);
    }

    ist_internal<int32_t> tree(keys_construct, 3);

    pasl::pctl::parray<int32_t> keys_remove = {-100, 2, 7, 8, 9, 13, 15, 21, 50, 75, 100, 103};
    std::vector<bool> expected_remove_res;
    for (uint64_t i = 0; i < keys_remove.size(); ++i)
    {
        auto it = keys_set.find(keys_remove[i]);
        if (it != keys_set.end())
        {
            expected_remove_res.push_back(true);
            keys_set.erase(it);
        }
        else
        {
            expected_remove_res.push_back(false);
        }
    }
    pasl::pctl::parray<bool> remove_res = tree.remove(keys_remove);
    ASSERT_EQ(keys_remove.size(), remove_res.size());
    for (uint64_t i = 0; i < remove_res.size(); ++i)
    {
        ASSERT_EQ(expected_remove_res[i], remove_res[i]);
    }

    for (int32_t i = -1000; i < 1000; ++i)
    {
        bool exp_res = keys_set.find(i) != keys_set.end();
        bool res = tree.contains(i);
        ASSERT_EQ(exp_res, res);
    }
}


TEST(remove, simple_batch_contains)
{
    pasl::pctl::raw raw_marker;
    pasl::pctl::parray<int32_t> keys_construct = {1, 2, 5, 7, 8, 9, 13, 15, 21, 50, 75, 100, 101, 102, 103};

    std::unordered_set<int32_t> keys_set;
    for (uint64_t i = 0; i < keys_construct.size(); ++i)
    {
        bool inserted = keys_set.insert(keys_construct[i]).second;
        ASSERT_TRUE(inserted);
    }

    ist_internal<int32_t> tree(keys_construct, 3);

    pasl::pctl::parray<int32_t> keys_remove = {-100, 2, 7, 8, 9, 13, 15, 21, 50, 75, 100, 103};
    std::vector<bool> expected_remove_res;
    for (uint64_t i = 0; i < keys_remove.size(); ++i)
    {
        auto it = keys_set.find(keys_remove[i]);
        if (it != keys_set.end())
        {
            expected_remove_res.push_back(true);
            keys_set.erase(it);
        }
        else
        {
            expected_remove_res.push_back(false);
        }
    }
    pasl::pctl::parray<bool> remove_res = tree.remove(keys_remove);
    ASSERT_EQ(keys_remove.size(), remove_res.size());
    for (uint64_t i = 0; i < remove_res.size(); ++i)
    {
        ASSERT_EQ(expected_remove_res[i], remove_res[i]);
    }

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

TEST(remove, empty_tree)
{
    pasl::pctl::raw raw_marker;

    ist_internal<int32_t> tree({}, 3);

    pasl::pctl::parray<int32_t> keys_remove = {-100, 2, 7, 8, 9, 13, 15, 21, 50, 75, 100, 103};
    pasl::pctl::parray<bool> remove_res = tree.remove(keys_remove);
    ASSERT_EQ(keys_remove.size(), remove_res.size());
    for (uint64_t i = 0; i < remove_res.size(); ++i)
    {
        ASSERT_FALSE(remove_res[i]);
    }

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

TEST(remove, empty_req)
{
    pasl::pctl::raw raw_marker;
    pasl::pctl::parray<int32_t> keys = {1, 7, 9, 100, 103, 105};
    ist_internal<int32_t> tree(keys, 3);

    pasl::pctl::parray<int32_t> req = {};
    pasl::pctl::parray<bool> contains_res = tree.remove(req);
    ASSERT_EQ(0, contains_res.size());
}

TEST(remove, single_remove_stress)
{
    uint32_t max_tree_size = 100'000;
    uint32_t max_req_size = 10'000;

    std::default_random_engine generator(time(nullptr));
    std::uniform_int_distribution<uint32_t> tree_size_distribution(1, max_tree_size);
    std::uniform_int_distribution<uint32_t> req_size_distribution(1, max_req_size);
    std::uniform_int_distribution<int32_t> elements_distribution(-100'000, 100'000);
    std::uniform_int_distribution<uint32_t> size_threshold_distribution(3, 10);

    for (uint32_t i = 0; i < TESTS_COUNT; ++i)
    {
        uint32_t cur_tree_size = tree_size_distribution(generator);
        uint32_t cur_size_threshold = size_threshold_distribution(generator);

        auto [keys_set, keys] = get_build_batch<int32_t>(cur_tree_size, generator, elements_distribution);
        ist_internal<int32_t> tree(keys, cur_size_threshold);

        uint32_t cur_remove_size = req_size_distribution(generator);
        auto [exp_remove_res, remove_keys] = get_remove_batch<int32_t>(
            keys_set, cur_remove_size, generator, elements_distribution
        );
        pasl::pctl::parray<bool> remove_res = tree.remove(remove_keys);
        ASSERT_EQ(exp_remove_res.size(), remove_res.size());
        for (uint64_t i = 0; i < remove_res.size(); ++i)
        {
            ASSERT_EQ(exp_remove_res[i], remove_res[i]);
        }

        uint32_t cur_contains_size = req_size_distribution(generator);
        auto [exp_contains_res, contains_keys] = get_contains_batch<int32_t>(
            keys_set, cur_contains_size, generator, elements_distribution
        );

        pasl::pctl::parray<bool> contains_res = tree.contains(contains_keys);
        ASSERT_EQ(exp_contains_res.size(), contains_res.size());
        for (uint64_t i = 0; i < contains_res.size(); ++i)
        {
            ASSERT_EQ(exp_contains_res[i], contains_res[i]);
        }
    }
}

TEST(remove, multiple_insert_stress)
{
    uint32_t max_tree_size = 100'000;
    uint32_t max_req_size = 10'000;

    std::default_random_engine generator(time(nullptr));
    std::uniform_int_distribution<uint32_t> tree_size_distribution(1, max_tree_size);
    std::uniform_int_distribution<uint32_t> req_size_distribution(1, max_req_size);
    std::uniform_int_distribution<int32_t> elements_distribution(-200'000, 200'000);
    std::uniform_int_distribution<uint32_t> size_threshold_distribution(3, 10);

    for (uint32_t i = 0; i < TESTS_COUNT; ++i)
    {
        uint32_t cur_tree_size = tree_size_distribution(generator);
        uint32_t cur_size_threshold = size_threshold_distribution(generator);

        auto [keys_set, keys] = get_build_batch<int32_t>(cur_tree_size, generator, elements_distribution);
        ist_internal<int32_t> tree(keys, cur_size_threshold);

        for (uint32_t j = 0; j < BATCH_REQUESTS_PER_TEST; ++j)
        {
            uint32_t cur_remove_size = req_size_distribution(generator);
            auto [exp_remove_res, remove_keys] = get_remove_batch<int32_t>(
                keys_set, cur_remove_size, generator, elements_distribution
            );
            pasl::pctl::parray<bool> remove_res = tree.remove(remove_keys);
            ASSERT_EQ(exp_remove_res.size(), remove_res.size());
            for (uint64_t i = 0; i < remove_res.size(); ++i)
            {
                ASSERT_EQ(exp_remove_res[i], remove_res[i]);
            }

            uint32_t cur_contains_size = req_size_distribution(generator);
            auto [exp_contains_res, contains_keys] = get_contains_batch<int32_t>(
                keys_set, cur_contains_size, generator, elements_distribution
            );

            pasl::pctl::parray<bool> contains_res = tree.contains(contains_keys);
            ASSERT_EQ(exp_contains_res.size(), contains_res.size());
            for (uint64_t i = 0; i < contains_res.size(); ++i)
            {
                ASSERT_EQ(exp_contains_res[i], contains_res[i]);
            }
        }
    }
}