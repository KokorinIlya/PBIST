#pragma once

#include "parray.hpp"
#include "datapar.hpp"
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

template <typename T>
bool is_sorted(pasl::pctl::parray<T> const& arr, bool is_strict)
{
    if (arr.size() == 0)
    {
        return true;
    }

    pasl::pctl::parray<bool> is_correct(
        arr.size() - 1, 
        [&arr, is_strict] (uint64_t i)
        {
            if (is_strict)
            {
                return arr[i] < arr[i + 1];
            }
            else
            {
                return arr[i] <= arr[i + 1];
            }
        }
    );
    return pasl::pctl::reduce(
        is_correct.begin(), is_correct.end(), true,
        [](bool a, bool b)
        {
            return a && b;
        }
    );
}