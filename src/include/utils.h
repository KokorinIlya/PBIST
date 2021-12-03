#ifndef IST_UTILS_H
#define IST_UTILS_H

#include "parray.hpp"
#include <iostream>

template <typename T>
void print_array(pasl::pctl::parray<T> const& arr)
{
    std::cout << "[";
    for (uint64_t i = 0; i < arr.size(); ++i)
    {
        std::cout << arr[i] << ", ";
    }
    std::cout << "]" << std::endl;
}

template <typename A>
void print_array(pasl::pctl::parray<std::pair<A, bool>> const& arr)
{
    std::cout << "[";
    for (uint64_t i = 0; i < arr.size(); ++i)
    {
        std::cout << arr[i].first << ": " << arr[i].second << ", ";
    }
    std::cout << "]" << std::endl;
}

#endif