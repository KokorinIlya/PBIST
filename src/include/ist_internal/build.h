#pragma once

#include "parray.hpp"
#include "datapar.hpp"
#include "ploop.hpp"
#include <cstdint>
#include <cmath>
#include <cassert>
#include <utility>
#include <memory>
#include <functional>
#include <iostream>
#include <string>

template <typename T>
struct ist_internal_node;

template <typename T>
std::unique_ptr<ist_internal_node<T>> do_build_from_keys(
    pasl::pctl::parray<T> const& keys, 
    uint64_t left, uint64_t right, uint64_t size_threshold)
{
    pasl::pctl::raw raw_marker;

    assert(right >= left);
    if (left == right)
    {
        return std::unique_ptr<ist_internal_node<T>>(nullptr);
    }

    uint64_t keys_count = right - left;

    if (keys_count <= size_threshold)
    {
        pasl::pctl::parray<std::pair<T, bool>> reps(
            keys_count,
            [left, &keys](long rep_idx) 
            {
                uint64_t key_idx = static_cast<uint64_t>(rep_idx + left);
                return std::make_pair(keys[key_idx], true);
            }
        );
        pasl::pctl::parray<std::unique_ptr<ist_internal_node<T>>> children(
            raw_marker, 0,
            [](long)
            {
                return nullptr;
            }
        );

        return std::make_unique<ist_internal_node<T>>(std::move(reps), std::move(children), keys_count);
    }

    uint64_t block_size = static_cast<uint64_t>(std::sqrt(keys_count));
    uint64_t rep_size = keys_count / (block_size + 1);

    pasl::pctl::parray<std::pair<T, bool>> reps(raw_marker, rep_size);
    pasl::pctl::parray<std::unique_ptr<ist_internal_node<T>>> children(raw_marker, rep_size + 1);

    // TODO: loop body is not constant, use Comp-based pfor
    pasl::pctl::parallel_for<uint64_t, std::function<void(long)>>(
        0, rep_size + 1,
        [block_size, size_threshold, left, right, rep_size, &reps, &children, &keys] (long child_idx)
        {
            if (child_idx < rep_size)
            {
                uint64_t start_idx = left + child_idx * (block_size + 1);
                uint64_t end_idx = start_idx + block_size;

                T const& cur_rep = keys[end_idx];
                new (&reps[child_idx]) std::pair<T, bool>(cur_rep, true);
                new (&children[child_idx]) std::unique_ptr<ist_internal_node<T>>(
                    do_build_from_keys(keys, start_idx, end_idx, size_threshold)
                );
            }
            else
            {
                assert(child_idx == rep_size);
                uint64_t start_idx = left + child_idx * (block_size + 1);
                uint64_t end_idx = right;
                new (&children[child_idx]) std::unique_ptr<ist_internal_node<T>>(
                    do_build_from_keys(keys, start_idx, end_idx, size_threshold)
                );
            }
        }
    );
    return std::make_unique<ist_internal_node<T>>(std::move(reps), std::move(children), keys_count);
}

template <typename T>
std::unique_ptr<ist_internal_node<T>> build_from_keys(pasl::pctl::parray<T> const& keys, uint64_t size_threshold)
{
    return do_build_from_keys(keys, 0, keys.size(), size_threshold);
}
