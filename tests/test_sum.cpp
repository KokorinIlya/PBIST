#include <gtest/gtest.h>
#include "sum.h"

TEST(sum, simple)
{
    pasl::pctl::parray<int32_t> xs = { 2, 5, 1, 7, 42, -24 };
    int32_t sum = calc_sum_parallel(xs);
    ASSERT_EQ(33, sum);
}