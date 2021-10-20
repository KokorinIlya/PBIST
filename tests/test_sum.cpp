#include <gtest/gtest.h>
#include "sum.h"
#include <cstdint>

TEST(sum, simple)
{
    pasl::pctl::parray<int> xs = { 2, 5, 1, 7, 42, -24 };
    int32_t sum = calc_sum(xs);
    ASSERT_EQ(33, sum);
}