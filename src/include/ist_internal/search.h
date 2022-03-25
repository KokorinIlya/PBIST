#pragma once

#include <cstdint>
#include "parray.hpp"
#include <utility>
#include <cassert>

template <typename T>
std::pair<uint64_t, bool> do_binary_search(
    pasl::pctl::parray<T> const& keys, T const& search_key,
    uint64_t left_border, uint64_t right_border,
    uint64_t* bin_ops_cnt_ptr)
{
    assert (0 <= left_border && left_border < right_border && right_border < keys.size());
    uint64_t cur_left = left_border;
    uint64_t cur_right = right_border;

    while (cur_left + 1 < cur_right)
    {
        assert(search_key > keys[cur_left]);
        assert(search_key <= keys[cur_right]);

        uint64_t cur_mid = cur_left + ((cur_right - cur_left) >> 1);
        ++(*bin_ops_cnt_ptr);
        std::cout << "BINOP" << std::endl;
        if (search_key > keys[cur_mid])
        {
            cur_left = cur_mid;
        }
        else
        {
            cur_right = cur_mid;
        }
    }

    assert(search_key > keys[cur_left]);
    assert(search_key <= keys[cur_right]);
    if (search_key == keys[cur_right])
    {
        return {cur_right, true};
    }
    else
    {
        return {cur_right, false};
    }
}

/*
result.second:
    - true -> key found in the keys array
    - false -> key not found in the keys array, can be found in the appropriate child subtree 

result.first:
    Index of the desired key in the keys array or index of the child subtree to continue the search
*/
template <typename T>
std::pair<uint64_t, bool> binary_search(
    pasl::pctl::parray<T> const& keys, T const& search_key,
    uint64_t* bin_ops_cnt_ptr, uint64_t* total_searches_ptr)
{
    ++(*total_searches_ptr);
    assert(keys.size() >= 1);
    if (search_key == keys[0])
    {
        return {0, true};
    }
    else if (search_key < keys[0])
    {
        return {0, false};
    }
    if (search_key > keys[keys.size() - 1])
    {
        return {keys.size(), false};
    }

    assert(keys.size() > 1);
    return do_binary_search(keys, search_key, 0, keys.size() - 1, bin_ops_cnt_ptr);
}

template <typename T>
std::pair<uint64_t, uint64_t> find_borders(
    pasl::pctl::parray<T> const& keys, T const& search_key,
    uint64_t initial_guess,
    uint64_t* exp_ops_cnt_ptr)
{
    assert(0 <= initial_guess && initial_guess < keys.size());
    assert(keys.size() > 1);

    if (keys[initial_guess] < search_key)
    {
        assert(initial_guess + 1 < keys.size());
        uint64_t left_border = initial_guess;
        uint64_t delta = 1;

        while (initial_guess + delta < keys.size() && keys[initial_guess + delta] < search_key)
        {
            left_border = initial_guess + delta;
            delta <<= 1;
            ++(*exp_ops_cnt_ptr);
        }

        if (initial_guess + delta < keys.size())
        {
            return {left_border, initial_guess + delta};
        }
        else
        {
            return {left_border, keys.size() - 1};
        }
    }
    else
    {
        assert(initial_guess >= 1);
        uint64_t right_border = initial_guess;
        uint64_t delta = 1;

        while (initial_guess >= delta && keys[initial_guess - delta] >= search_key)
        {
            right_border = initial_guess - delta;
            delta <<= 1;
            ++(*exp_ops_cnt_ptr);
        }

        if (initial_guess >= delta)
        {
            return {initial_guess - delta, right_border};
        }
        else
        {
            return {0, right_border};
        }
    }
}

template <typename T>
std::pair<uint64_t, bool> interpolation_search(
    pasl::pctl::parray<T> const& keys, T const& search_key,
    pasl::pctl::parray<uint64_t> const& id,
    uint64_t* exp_ops_cnt_ptr, uint64_t* bin_ops_cnt_ptr, uint64_t* total_searches_ptr)
{
    ++(*total_searches_ptr);
    assert(keys.size() >= 1);
    T const& min = keys[0];
    T const& max = keys[keys.size() - 1];

    if (search_key == keys[0])
    {
        return {0, true};
    }
    else if (search_key < keys[0])
    {
        return {0, false};
    }
    if (search_key > keys[keys.size() - 1])
    {
        return {keys.size(), false};
    }
    else if (search_key == keys[keys.size() - 1])
    {
        return {keys.size() - 1, true};
    }

    assert(keys.size() > 1);
    double frac = (static_cast<double>(search_key) - min) / (max - min);
    assert(0 < frac && frac < 1);
    uint64_t id_idx = static_cast<uint64_t>(frac * id.size());
    assert(0 <= id_idx && id_idx < id.size());
    uint64_t initial_guess = id[id_idx];
    
    auto [left_border, right_border] = find_borders(keys, search_key, initial_guess, exp_ops_cnt_ptr);
    if (left_border + 1 < right_border)
    {
        std::cout << "LARGE: " << left_border << " " << right_border << std::endl;
    }
    return do_binary_search(keys, search_key, left_border, right_border, bin_ops_cnt_ptr);
}
