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
#include "utils.h"
#include <algorithm>

template <typename T>
struct ist_internal_node;

template <typename T>
std::unique_ptr<ist_internal_node<T>> do_build_from_keys(
    pasl::pctl::parray<T> const& keys, 
    uint64_t left, uint64_t right, uint64_t size_threshold);

template <typename T>
void do_build_single_cell(
    pasl::pctl::parray<std::unique_ptr<ist_internal_node<T>>>& children,
    pasl::pctl::parray<std::pair<T, bool>>& reps,
    pasl::pctl::parray<T> const& keys,
    uint64_t block_size, uint64_t rep_size, uint64_t size_threshold, 
    uint64_t left, uint64_t right, uint64_t child_idx)
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
            raw_marker, keys_count,
            [left, &keys](uint64_t rep_idx) 
            {
                uint64_t key_idx = static_cast<uint64_t>(rep_idx + left);
                return std::make_pair(keys[key_idx], true);
            }
        );
        pasl::pctl::parray<std::unique_ptr<ist_internal_node<T>>> children(raw_marker, 0);

        return std::make_unique<ist_internal_node<T>>(std::move(reps), std::move(children), keys_count);
    }

    uint64_t block_size = static_cast<uint64_t>(std::sqrt(keys_count));
    uint64_t rep_size = keys_count / (block_size + 1);

    pasl::pctl::parray<std::pair<T, bool>> reps(raw_marker, rep_size);
    pasl::pctl::parray<std::unique_ptr<ist_internal_node<T>>> children(raw_marker, rep_size + 1);

    pasl::pctl::range::parallel_for(
        static_cast<uint64_t>(0), static_cast<uint64_t>(rep_size + 1),
        [left, right, block_size, rep_size] (uint64_t left_idx, uint64_t right_idx)
        {
            assert(0 <= left_idx && left_idx < right_idx && right_idx <= rep_size + 1);

            if (right_idx < rep_size + 1)
            {
                /*
                start_idx = left + left_idx  * (block_size + 1);
                end_idx   = left + right_idx * (block_size + 1);
                */
                return (block_size + 1) * (right_idx - left_idx);
            }
            else
            {
                assert(right_idx == rep_size + 1);
                uint64_t start_idx = left + left_idx * (block_size + 1);
                return right - start_idx;
            }
        },
        [&children, &reps, &keys, block_size, rep_size, size_threshold, left, right] (uint64_t child_idx)
        {
            do_build_single_cell(
                children, reps, keys, block_size, rep_size, 
                size_threshold, left, right, child_idx
            );
        },
        [
            &children, &reps, &keys, 
            block_size, rep_size, size_threshold, left, right
        ](uint64_t left_idx, uint64_t right_idx)
        {
            assert(0 <= left_idx && left_idx < right_idx && right_idx <= rep_size + 1);
            for (uint64_t child_idx = left_idx; child_idx < right_idx; ++child_idx)
            {
                do_build_single_cell(
                    children, reps, keys, block_size, rep_size, 
                    size_threshold, left, right, child_idx
                );
            }
        }
    );
    return std::make_unique<ist_internal_node<T>>(std::move(reps), std::move(children), keys_count);
}

template <typename T>
std::unique_ptr<ist_internal_node<T>> build_from_keys(pasl::pctl::parray<T> const& keys, uint64_t size_threshold)
{
    assert(is_sorted(keys, true));
    return do_build_from_keys(keys, 0, keys.size(), size_threshold);
}

template <typename T>
pasl::pctl::parray<uint64_t> build_id(pasl::pctl::parray<T> const& keys, uint64_t id_size)
{
    assert(keys.size() >= 1);

    double min = static_cast<double>(keys[0]);
    double max = static_cast<double>(keys[keys.size() - 1]);
    double range = max - min;

    return pasl::pctl::parray<uint64_t>(
        pasl::pctl::raw{}, id_size,
        [&keys, min, range, id_size](uint64_t i)
        {
            double frac = min + range * static_cast<double>(i) / static_cast<double>(id_size);
            typename pasl::pctl::parray<T>::iterator it = std::lower_bound(keys.begin(), keys.end(), frac);
            assert(it != keys.end());
            return it - keys.begin();
        }
    );
}