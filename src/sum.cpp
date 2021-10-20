#include "datapar.hpp"
#include "sum.h"
#include <cstdint>

int32_t calc_sum(pasl::pctl::parray<int32_t> const& arr) 
{
	return pasl::pctl::sum(xs.begin(), xs.end());
}
