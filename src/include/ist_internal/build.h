#ifndef IST_INTERNAL_BUILDING_H
#define IST_INTERNAL_BUILDING_H

#include "parray.hpp"
#include <cstdint>
#include <cmath>
#include <cassert>
#include <utility>
#include <memory>

template <typename T>
std::unique_ptr<ist_internal_node<T>> build_from_keys(pasl::pctl::parray<T> const& keys, uint64_t size_threshold)
{
    return do_build_from_keys(keys, 0, keys.size(), size_threshold);
}

template <typename T>
std::unique_ptr<ist_internal_node<T>> do_build_from_keys(
    pasl::pctl::parray<T> const& keys, 
    uint64_t left, uint64_t right, uint64_t size_threshold)
{
    assert(right >= left);
    if (left == right)
    {
        return std::unique_ptr<ist_internal_node<T>>(nullptr);
    }

    uint64_t keys_count = right - left;

    if (keys_count <= size_threshold)
    {
        pasl::pctl::parray<std::pair<T, bool>> rep(
            keys_count,
            [left, &keys](long rep_idx) 
            {
                uint64_t key_idx = static_cast<uint64_t>(rep_idx + left);
                return {keys[key_idx], true};
            }
        );
        pasl::pctl::parray<std::unique_ptr<ist_internal_node<T>>> children;

        return std::make_shared<ist_internal_node<T>>(std::move(rep), std::move(children));
    }

    uint64_t block_size = static_cast<uint64_t>(std::sqrt(keys_count));
    uint64_t rep_size = keys_count / (block_size + 1);
    pasl::pctl::parray<std::pair<T, bool>> reps(rep_size);
    pasl::pctl::parray<std::unique_ptr<ist_internal_node<T>>> children(rep_size + 1);
}

template <typename T>
pasl::pctl::parray<std::pair<T, bool>> do_build_rep(pasl::pctl::parray<T> const& keys,
    uint64_t left, uint64_t right)
{
    assert(right > left);
    uint64_t keys_count = right - left;
    uint64_t block_size = static_cast<uint64_t>(std::sqrt(keys_count));
    uint64_t rep_size = keys_count / (block_size + 1);
    return pasl::pctl::parray<std::pair<T, bool>>(
        rep_size,
        [block_size](long rep_idx)
        {
            uint64_t key_idx = static_cast<uint64_t>((rep_idx + 1) * block_size + rep_idx);
            return {keys[key_idx], true}; 
        }
    )
}

#endif