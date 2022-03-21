#pragma once

#include <cstdint>
#include "parray.hpp"
#include <utility>
#include <cassert>

template <typename T>
std::pair<uint64_t, bool> do_binary_search(
    pasl::pctl::parray<std::pair<T, bool>> const& keys, 
    T const& search_key,
    uint64_t left_border, uint64_t right_border)
{
    assert (0 <= left_border && left_border < right_border && right_border < keys.size());
    uint64_t cur_left = left_border;
    uint64_t cur_right = right_border;

    while (cur_left + 1 < cur_right)
    {
        assert(search_key > keys[cur_left].first);
        assert(search_key <= keys[cur_right].first);

        uint64_t cur_mid = cur_left + ((cur_right - cur_left) >> 1);
        if (search_key > keys[cur_mid].first)
        {
            cur_left = cur_mid;
        }
        else
        {
            cur_right = cur_mid;
        }
    }

    assert(search_key > keys[cur_left].first);
    assert(search_key <= keys[cur_right].first);
    if (search_key == keys[cur_right].first)
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
    pasl::pctl::parray<std::pair<T, bool>> const& keys, 
    T const& search_key)
{
    assert(keys.size() >= 1);
    if (search_key == keys[0].first)
    {
        return {0, true};
    }
    else if (search_key < keys[0].first)
    {
        return {0, false};
    }
    if (search_key > keys[keys.size() - 1].first)
    {
        return {keys.size(), false};
    }

    assert(keys.size() > 1);
    return do_binary_search(keys, search_key, 0, keys.size() - 1);
}

template <typename T>
std::pair<uint64_t, uint64_t> find_borders(
    pasl::pctl::parray<std::pair<T, bool>> const& keys, 
    T const& search_key,
    uint64_t initial_guess)
{
    assert(0 <= initial_guess && initial_guess < keys.size());
    assert(keys.size() > 1);

    if (keys[initial_guess].first < search_key)
    {
        assert(initial_guess + 1 < keys.size());
        uint64_t left_border = initial_guess;
        uint64_t delta = 1;

        while (initial_guess + delta < keys.size() && keys[initial_guess + delta].first < search_key)
        {
            left_border = initial_guess + delta;
            delta <<= 1;
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

        while (initial_guess >= delta && keys[initial_guess - delta].first >= search_key)
        {
            right_border = initial_guess - delta;
            delta <<= 1;
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
    pasl::pctl::parray<std::pair<T, bool>> const& keys, 
    T const& search_key,
    pasl::pctl::parray<uint64_t> const& id)
{
    //std::cout << "SEARCH " << search_key << std::endl;
    assert(keys.size() >= 1);
    T const& min = keys[0].first;
    T const& max = keys[keys.size() - 1].first;

    if (search_key == keys[0].first)
    {
        return {0, true};
    }
    else if (search_key < keys[0].first)
    {
        return {0, false};
    }
    if (search_key > keys[keys.size() - 1].first)
    {
        return {keys.size(), false};
    }
    else if (search_key == keys[keys.size() - 1].first)
    {
        return {keys.size() - 1, true};
    }

    assert(keys.size() > 1);
    double frac = (static_cast<double>(search_key) - min) / (max - min);
    assert(0 < frac && frac < 1);
    uint64_t id_idx = static_cast<uint64_t>(frac * id.size());
    assert(0 <= id_idx && id_idx < id.size());
    uint64_t initial_guess = id[id_idx];
    //std::cout << "frac = " << frac << "; id_idx = " << id_idx << "; init_g = " << initial_guess << std::endl;
    
    auto [left_border, right_border] = find_borders(keys, search_key, initial_guess);
    return do_binary_search(keys, search_key, left_border, right_border);
}
