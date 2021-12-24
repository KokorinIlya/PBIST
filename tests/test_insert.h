#pragma once

#include <gtest/gtest.h>
#include "ist_internal/tree.h"
#include <random>
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
    pasl::pctl::parray<bool> insert_res = tree.insert(keys);
    ASSERT_EQ(keys.size(), insert_res.size());
    for (uint64_t i = 0; i < insert_res.size(); ++i)
    {
        ASSERT_TRUE(insert_res[i]);
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

    pasl::pctl::parray<int32_t> keys_insert = {-100, 2, 7, 8, 9, 13, 15, 21, 50, 75, 100, 103};
    std::vector<bool> expected_insert_res;
    for (uint64_t i = 0; i < keys_insert.size(); ++i)
    {
        bool inserted = keys_set.insert(keys_insert[i]).second;
        expected_insert_res.push_back(inserted);
    }
    pasl::pctl::parray<bool> insert_res = tree.insert(keys_insert);
    ASSERT_EQ(keys_insert.size(), insert_res.size());
    for (uint64_t i = 0; i < insert_res.size(); ++i)
    {
        ASSERT_EQ(expected_insert_res[i], insert_res[i]);
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

TEST(insert, empty_req)
{
    pasl::pctl::raw raw_marker;
    pasl::pctl::parray<int32_t> keys = {1, 7, 9, 100, 103, 105};
    ist_internal<int32_t> tree(keys, 3);

    pasl::pctl::parray<int32_t> req = {};
    pasl::pctl::parray<bool> contains_res = tree.insert(req);
    ASSERT_EQ(0, contains_res.size());
}

void do_test_insert_stress(
    uint32_t TESTS_COUNT, uint32_t REQ_PER_TEST, uint32_t MAX_INITIAL_TREE_SIZE, uint32_t MAX_REQ_SIZE,
    int32_t KEYS_FROM, int32_t KEYS_TO, bool CHECK_SIZES)
{
    std::default_random_engine generator(time(nullptr));
    std::uniform_int_distribution<uint32_t> req_size_distribution(1, MAX_REQ_SIZE);
    std::uniform_int_distribution<int32_t> elements_distribution(KEYS_FROM, KEYS_TO);
    std::uniform_int_distribution<uint32_t> size_threshold_distribution(3, 10);

    for (uint32_t i = 0; i < TESTS_COUNT; ++i)
    {        
        pasl::pctl::parray<int32_t> keys = {};
        std::unordered_set<int32_t> keys_set;
        if (MAX_INITIAL_TREE_SIZE != 0)
        {
            std::uniform_int_distribution<uint32_t> tree_size_distribution(1, MAX_INITIAL_TREE_SIZE);
            uint32_t cur_tree_size = tree_size_distribution(generator);
            auto buil_batch = get_build_batch<int32_t>(cur_tree_size, generator, elements_distribution);
            keys = buil_batch.second;
            keys_set = buil_batch.first;
        }
        
        uint32_t cur_size_threshold = size_threshold_distribution(generator);
        ist_internal<int32_t> tree(keys, cur_size_threshold);

        for (uint32_t j = 0; j < REQ_PER_TEST; ++j)
        {
            uint32_t cur_insert_size = req_size_distribution(generator);
            auto [exp_insert_res, insert_keys] = get_insert_batch<int32_t>(
                keys_set, cur_insert_size, generator, elements_distribution
            );
            pasl::pctl::parray<bool> insert_res = tree.insert(insert_keys);
            ASSERT_EQ(exp_insert_res.size(), insert_res.size());
            for (uint64_t i = 0; i < insert_res.size(); ++i)
            {
                ASSERT_EQ(exp_insert_res[i], insert_res[i]);
            }

            if (CHECK_SIZES)
            {
                tree.calc_tree_size();
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

TEST(insert, insert_empty_stress)
{
    uint32_t MAX_INITIAL_TREE_SIZE = 0;
    uint32_t MAX_REQ_SIZE = 100'000;
    uint32_t TESTS_COUNT = 200;
    uint32_t REQ_PER_TEST = 1;
    int32_t KEYS_FROM = -100'000;
    int32_t KEYS_TO = 100'000;

    do_test_insert_stress(
        TESTS_COUNT, REQ_PER_TEST, MAX_INITIAL_TREE_SIZE, MAX_REQ_SIZE,
        KEYS_FROM, KEYS_TO, false
    );
}

TEST(insert, single_insert_non_empty_stress)
{
    uint32_t MAX_INITIAL_TREE_SIZE = 100'000;
    uint32_t MAX_REQ_SIZE = 100'000;
    uint32_t TESTS_COUNT = 200;
    uint32_t REQ_PER_TEST = 1;
    int32_t KEYS_FROM = -200'000;
    int32_t KEYS_TO = 200'000;

    do_test_insert_stress(
        TESTS_COUNT, REQ_PER_TEST, MAX_INITIAL_TREE_SIZE, MAX_REQ_SIZE,
        KEYS_FROM, KEYS_TO, false
    );
}

TEST(insert, multiple_insert_stress)
{
    uint32_t MAX_INITIAL_TREE_SIZE = 100'000;
    uint32_t MAX_REQ_SIZE = 100'000;
    uint32_t TESTS_COUNT = 100;
    uint32_t REQ_PER_TEST = 5;
    int32_t KEYS_FROM = -1'000'000;
    int32_t KEYS_TO = 1'000'000;

    do_test_insert_stress(
        TESTS_COUNT, REQ_PER_TEST, MAX_INITIAL_TREE_SIZE, MAX_REQ_SIZE,
        KEYS_FROM, KEYS_TO, false
    );
}

TEST(insert, insert_stress_check_sizes)
{
    uint32_t MAX_INITIAL_TREE_SIZE = 10'000;
    uint32_t MAX_REQ_SIZE = 10'000;
    uint32_t TESTS_COUNT = 10;
    uint32_t REQ_PER_TEST = 3;
    int32_t KEYS_FROM = -50'000;
    int32_t KEYS_TO = 50'000;

    do_test_insert_stress(
        TESTS_COUNT, REQ_PER_TEST, MAX_INITIAL_TREE_SIZE, MAX_REQ_SIZE,
        KEYS_FROM, KEYS_TO, true
    );
}