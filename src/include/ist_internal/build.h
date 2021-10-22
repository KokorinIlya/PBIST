#ifndef IST_INTERNAL_BUILDING_H
#define IST_INTERNAL_BUILDING_H

#include "parray.hpp"
#include "datapar.hpp"
#include "ploop.hpp"
#include <cstdint>
#include <cmath>
#include <cassert>
#include <utility>
#include <memory>
#include "node.h"
#include <functional>
#include <iostream>
#include <string>

template <typename T>
std::unique_ptr<ist_internal_node<T>> do_build_from_keys(
    pasl::pctl::parray<T> const& keys, 
    uint64_t left, uint64_t right, uint64_t size_threshold)
{
    std::cout << "left = " << left << ", right = " << right << std::endl;
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
            0,
            [](long)
            {
                return std::unique_ptr<ist_internal_node<T>>(nullptr);
            }
        );

        return std::make_unique<ist_internal_node<T>>(std::move(reps), std::move(children));
    }

    std::cout << "Inner node" << std::endl;
    uint64_t block_size = static_cast<uint64_t>(std::sqrt(keys_count));
    uint64_t rep_size = keys_count / (block_size + 1);
    std::cout << "block_size = " << block_size << ", rep_size = " << rep_size << std::endl;

    // TODO: use raw allocated memory
    pasl::pctl::parray<std::pair<T, bool>> reps(rep_size);
    std::cout << "reps.size() = " << reps.size() << std::endl;
    pasl::pctl::parray<std::unique_ptr<ist_internal_node<T>>> children(
        rep_size + 1,
        [](long)
        {
            return std::unique_ptr<ist_internal_node<T>>(nullptr);
        }
    );
    std::cerr << "children.size() = " << children.size() << std::endl;

    std::cout << "Before loop" << std::endl;
    std::function<void(long)> loop_body = 
        [block_size, size_threshold, right, rep_size, &reps, &children, &keys] (long child_idx)
    {
        if (child_idx <= rep_size)
        {
            uint64_t start_idx;
            uint64_t end_idx;

            if (child_idx == 0)
            {
                start_idx = 0;
                end_idx = block_size;
            }
            else
            {
                start_idx = (child_idx - 1) * (block_size + 1) + block_size;
                end_idx = child_idx * (block_size + 1) + block_size;
            }
            std::cout << "start_idx = " << start_idx << ", end_idx = " << end_idx << std::endl;

            T const& cur_rep = keys[end_idx];
            reps[child_idx] = {cur_rep, true};
            children[child_idx] = do_build_from_keys(keys, start_idx, end_idx, size_threshold);;

        }
        else
        {
            assert(child_idx == rep_size);
            uint64_t start_idx = (child_idx - 1) * (block_size + 1) + block_size;
            uint64_t end_idx = right;
            std::cout << "start_idx = " << start_idx << ", end_idx = " << end_idx << std::endl;
            children[child_idx] = do_build_from_keys(keys, start_idx, end_idx, size_threshold);
        }
    };

    // TODO: loop body is not constant, use Comp-based pfor
    pasl::pctl::parallel_for<uint64_t, std::function<void(long)>>(0, rep_size + 1, loop_body);
    return std::make_unique<ist_internal_node<T>>(std::move(reps), std::move(children));
}

template <typename T>
std::unique_ptr<ist_internal_node<T>> build_from_keys(pasl::pctl::parray<T> const& keys, uint64_t size_threshold)
{
    return do_build_from_keys(keys, 0, keys.size(), size_threshold);
}
#endif