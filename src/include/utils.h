#ifndef IST_UTILS_H
#define IST_UTILS_H

#include "parray.hpp"
#include <iostream>

template <typename T>
inline void print_array(pasl::pctl::parray<T> const& arr)
{
    std::cout << "[";
    for (uint64_t i = 0; i < arr.size(); ++i)
    {
        std::cout << arr[i] << ", ";
    }
    std::cout << "]" << std::endl;
}

#endif