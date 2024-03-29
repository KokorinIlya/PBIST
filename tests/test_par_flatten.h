#pragma once

#include <gtest/gtest.h>
#include "ist_internal/node.h"
#include "ist_internal/build.h"
#include <memory>
#include <cstdint>
#include <unistd.h>
#include <random>
#include <algorithm>
#include <unordered_set>
#include "test_utils.h"

TEST(parallel_flatten, simple)
{
    pasl::pctl::parray<int32_t> keys(
        10,
        [](long i)
        {
            return static_cast<int32_t>(i);
        }
    );
    ist_internal_node<int32_t>* result = build_from_keys(keys, 3);
    auto par_flattened_keys = result->get_keys();
    pasl::pctl::parray<int32_t> exp_par_flattened_keys = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    ASSERT_EQ(exp_par_flattened_keys.size(), par_flattened_keys.size());
    for (uint64_t i = 0; i < par_flattened_keys.size(); ++i)
    {
        ASSERT_EQ(exp_par_flattened_keys[i], par_flattened_keys[i]);
    }
    delete result;
}

TEST(parallel_flatten, stress) 
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
        auto flattened_keys = result->get_keys();
        ASSERT_EQ(tree_keys.size(), flattened_keys.size());
        for (uint64_t i = 0; i < flattened_keys.size(); ++i)
        {
            ASSERT_EQ(tree_keys[i], flattened_keys[i]);
        }
        delete result;
    }
}
