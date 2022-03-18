#pragma once

#include <gtest/gtest.h>
#include "ist_internal/build.h"
#include <cstdint>
#include <iostream>
#include <utility>

TEST(id_building, simple)
{
    pasl::pctl::parray<std::pair<uint64_t, bool>> keys = {
        {1, true}, {2, true}, {5, true}, {7, true}, {9, true}, {11, true}, {14, true}, {32, true}, {64, true}
    };
    pasl::pctl::parray<uint64_t> id = build_id(keys, keys.size());
    pasl::pctl::parray<uint64_t> exp_id = {0, 4, 7, 7, 7, 8, 8, 8, 8};
    ASSERT_EQ(exp_id.size(), id.size());
    for (uint32_t i = 0; i < id.size(); ++i)
    {
        ASSERT_EQ(exp_id[i], id[i]);
    }
}

TEST(id_building, stress) 
{
    uint32_t MAX_SIZE = 100'000;
    uint32_t TESTS_COUNT = 200;
    int32_t KEYS_FROM = -1'000'000;
    int32_t KEYS_TO = 1'000'000;

    std::default_random_engine generator(time(nullptr));
    std::uniform_int_distribution<uint32_t> size_distribution(1, MAX_SIZE);
    std::uniform_int_distribution<int32_t> elements_distribution(KEYS_FROM, KEYS_TO);

    for (uint32_t i = 0; i < TESTS_COUNT; ++i)
    {
        uint32_t cur_keys_size = size_distribution(generator);
        uint32_t cur_id_size = size_distribution(generator);
        pasl::pctl::parray<int32_t> ks = 
            get_build_batch<int32_t>(cur_keys_size, generator, elements_distribution).second;
        pasl::pctl::parray<std::pair<int32_t, bool>> keys(
            pasl::pctl::raw{}, ks.size(),
            [&ks](uint64_t j) -> std::pair<int32_t, bool>
            {
                return {ks[j], true};
            }
        );

        double min = static_cast<double>(keys[0].first);
        double max = static_cast<double>(keys[keys.size() - 1].first);
        double range = max - min;

        pasl::pctl::parray<uint64_t> id = build_id(keys, cur_id_size);
        ASSERT_EQ(cur_id_size, id.size());

        for (uint32_t j = 0; j < cur_id_size; ++j)
        {
            double frac = min + range * static_cast<double>(j) / static_cast<double>(cur_id_size);

            uint32_t result = 200'000;
            for (uint32_t k = 0; k < keys.size(); ++k)
            {
                if (frac <= keys[k].first)
                {
                    result = k;
                    break;
                }
            }
            ASSERT_TRUE(result < keys.size());
            ASSERT_EQ(result, id[j]);
        }
    }
}