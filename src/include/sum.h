#ifndef PARALLEL_SUM_H
#define PARALLEL_SUM_H

#include "datapar.hpp"
#include <cstdint>

template <typename T>
T calc_sum_parallel(pasl::pctl::parray<T> const& arr) 
{
	return pasl::pctl::sum(arr.begin(), arr.end());
}

#endif