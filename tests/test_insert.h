#pragma once

#include <gtest/gtest.h>
#include "ist_internal/tree.h"
#include <random>
#include "config.h"
#include <unordered_set>
#include "utils.h"
#include <unordered_set>

TEST(insert, insert_empty_simple)
{
    pasl::pctl::raw raw_marker;
    pasl::pctl::parray<int32_t> keys = {1, 2, 5, 7, 8, 9, 13, 15, 21, 50, 75, 100, 101, 102, 103};
    std::unordered_set<int32_t> keys_set;
    for (uint64_t i = 0; i < keys.size(); ++i)
    {
        bool inserted = keys_set.insert(keys[i]).second;
        ASSERT_TRUE(inserted);
    }

    ist_internal<int32_t> tree({}, 3);
    tree.insert(keys);

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

TEST(insert, insert_non_empty_simple)
{
    pasl::pctl::raw raw_marker;
    pasl::pctl::parray<int32_t> keys_construct = {1, 2, 5, 8, 21, 50, 101, 102};

    std::unordered_set<int32_t> keys_set;
    for (uint64_t i = 0; i < keys_construct.size(); ++i)
    {
        bool inserted = keys_set.insert(keys_construct[i]).second;
        ASSERT_TRUE(inserted);
    }

    ist_internal<int32_t> tree(keys_construct, 3);

    pasl::pctl::parray<int32_t> keys_insert = {7, 9, 13, 15, 75, 100, 103};
    for (uint64_t i = 0; i < keys_insert.size(); ++i)
    {
        bool inserted = keys_set.insert(keys_insert[i]).second;
        ASSERT_TRUE(inserted);
    }
    tree.insert(keys_insert);

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

TEST(insert, insert_empty_stress)
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

        std::vector<int32_t> keys_v;
        std::unordered_set<int32_t> keys_set;
        while (keys_v.size() < cur_tree_size)
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
        pasl::pctl::parray<int32_t> keys(
            keys_v.size(),
            [&keys_v](long i)
            {
                return keys_v[i];
            }
        );
        ist_internal<int32_t> tree({}, cur_size_threshold);
        tree.insert(keys);

        uint32_t cur_req_size = req_size_distribution(generator);
        std::vector<int32_t> req_keys_v;
        std::unordered_set<int32_t> req_keys_set;
        while (req_keys_v.size() < cur_req_size)
        {
            assert(req_keys_v.size() == req_keys_set.size());
            int32_t cur_elem =  elements_distribution(generator);
            if (req_keys_set.find(cur_elem) != req_keys_set.end())
            {
                continue;
            }
            auto insert_res = req_keys_set.insert(cur_elem);
            assert(insert_res.second);
            req_keys_v.push_back(cur_elem);
        }
        assert(req_keys_v.size() == req_keys_set.size());

        std::sort(req_keys_v.begin(), req_keys_v.end());
        pasl::pctl::parray<int32_t> req_keys(
            req_keys_v.size(),
            [&req_keys_v](long i)
            {
                return req_keys_v[i];
            }
        );

        pasl::pctl::parray<bool> expected_res(
            req_keys_v.size(),
            [&req_keys_v, &keys_set](long i)
            {
                return keys_set.find(req_keys_v[i]) != keys_set.end();
            }
        );

        pasl::pctl::parray<bool> contains_res = tree.contains(req_keys);
        ASSERT_EQ(expected_res.size(), contains_res.size());
        for (uint64_t i = 0; i < contains_res.size(); ++i)
        {
            ASSERT_EQ(expected_res[i], contains_res[i]);
        }
    }
}