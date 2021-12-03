#pragma once

#include <gtest/gtest.h>
#include "ist_internal/tree.h"
#include <random>
#include "config.h"
#include <unordered_set>
#include "utils.h"

TEST(contains_batch, simple)
{
    pasl::pctl::parray<int32_t> keys = {1, 2, 5, 7, 8, 9, 13, 15, 21, 50, 75, 100, 101, 102, 103};
    ist_internal<int32_t> tree(keys, 3);
    pasl::pctl::parray<int32_t> keys_to_check = {
        -100, 1, 2, 4, 5, 6, 7, 8, 9, 11, 13, 14, 15, 17, 19, 21, 25, 26, 50, 60, 61, 62, 75, 100, 101, 102, 103, 200, 201
    };
    pasl::pctl::parray<bool> contains_res = tree.contains(keys_to_check);
    print_array(contains_res);

}