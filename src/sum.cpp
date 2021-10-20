#include "datapar.hpp"
#include "sum.h"
#include <cstdint>

int32_t calc_sum_parallel(pasl::pctl::parray<int32_t> const& arr) 
{
	return pasl::pctl::sum(arr.begin(), arr.end());
}