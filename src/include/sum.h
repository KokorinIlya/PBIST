#ifndef PARALLEL_SUM_H
#define PARALLEL_SUM_H

#include "datapar.hpp"
#include <cstdint>

int32_t calc_sum_parallel(pasl::pctl::parray<int32_t> const& arr);

#endif