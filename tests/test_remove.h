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
