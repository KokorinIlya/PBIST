#pragma once

#include <gtest/gtest.h>
#include "ist_internal/tree.h"
#include <random>
#include "config.h"
#include <unordered_set>
#include "utils.h"
#include <unordered_set>
#include "test_utils.h"

TEST(all_operations, stress)
{
    uint32_t max_tree_size = 100'000;
    uint32_t max_req_size = 100'000;

    std::default_random_engine generator(time(nullptr));
    std::uniform_int_distribution<uint32_t> tree_size_distribution(1, max_tree_size);
    std::uniform_int_distribution<uint32_t> req_size_distribution(1, max_req_size);
    std::uniform_int_distribution<int32_t> elements_distribution(-200'000, 00'000);
    std::uniform_int_distribution<uint32_t> size_threshold_distribution(3, 10);
    std::uniform_int_distribution<uint32_t> req_type_distribution(1, 3);

    for (uint32_t i = 0; i < TESTS_COUNT; ++i)
    {
        uint32_t cur_tree_size = tree_size_distribution(generator);
        uint32_t cur_size_threshold = size_threshold_distribution(generator);

        auto [tree_keys_set, tree_keys] = get_build_batch<int32_t>(
            cur_tree_size, generator, elements_distribution
        );

        ist_internal<int32_t> tree(tree_keys, cur_size_threshold);

        for (uint32_t j = 0; j < REQUESTS_PER_TEST / 10; ++j)
        {
            uint32_t cur_req_size = req_size_distribution(generator);
            uint32_t cur_req_type = req_type_distribution(generator);

            if (cur_req_type == 1) // Insert
            {
                auto [exp_insert_res, insert_keys] = get_insert_batch<int32_t>(
                    tree_keys_set, cur_req_size, generator, elements_distribution
                );
                pasl::pctl::parray<bool> insert_res = tree.insert(insert_keys);
                ASSERT_EQ(exp_insert_res.size(), insert_res.size());
                for (uint64_t i = 0; i < insert_res.size(); ++i)
                {
                    ASSERT_EQ(exp_insert_res[i], insert_res[i]);
                }
            }
            else if (cur_req_type == 2) // Remove
            {
                auto [exp_remove_res, remove_keys] = get_remove_batch<int32_t>(
                    tree_keys_set, cur_req_size, generator, elements_distribution
                );
                pasl::pctl::parray<bool> remove_res = tree.remove(remove_keys);
                ASSERT_EQ(exp_remove_res.size(), remove_res.size());
                for (uint64_t i = 0; i < remove_res.size(); ++i)
                {
                    ASSERT_EQ(exp_remove_res[i], remove_res[i]);
                }
            }
            else // Contains
            {
                assert(cur_req_type = 3);
                auto [exp_contains_res, contains_keys] = get_contains_batch<int32_t>(
                    tree_keys_set, cur_req_size, generator, elements_distribution
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
}