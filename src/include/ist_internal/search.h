#pragma once

#include <cstdint>
#include "parray.hpp"
#include <utility>
#include <cassert>

/*
result.second:
    - true -> key found in the keys array
    - false -> key not found in the keys array, can be found in the appropriate child subtree 

result.first:
    Index of the desired key in the keys array or index of the child subtree to continue search
*/
template <typename T>
std::pair<uint64_t, bool> binary_search(pasl::pctl::parray<std::pair<T, bool>> const& keys, T const& search_key)
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

    uint64_t cur_left = 0;
    uint64_t cur_right = keys.size() - 1;
    assert(search_key > keys[cur_left].first);
    assert(search_key <= keys[cur_right].first);

    while (cur_left + 1 < cur_right)
    {
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
