#pragma once

#include "node.h"
#include "build.h"
#include <memory>
#include "search.h"
#include <cassert>
#include "parray.hpp"
#include "datapar.hpp"

template <typename T>
struct ist_internal
{
public:
    ist_internal(pasl::pctl::parray<T> const& keys, uint64_t size_threshold)
        : root(build_from_keys(keys, size_threshold)), _size_threshold(size_threshold)
    {}

    bool contains(T const& search_key) const
    {
        ist_internal_node<T> const* cur_node = root.get();
        while (true)
        {
            if (cur_node == nullptr)
            {
                return false;
            }
            auto [idx, found] = binary_search(cur_node->keys, search_key);
            if (found)
            {
                assert(cur_node->keys[idx].first == search_key);
                return cur_node->keys[idx].second;
            }
            else
            {
                if (cur_node->is_terminal())
                {
                    assert(cur_node->children.size() == 0);
                    return false;
                }
                else
                {
                    assert(cur_node->children.size() > 0);
                    assert(0 <= idx && idx < cur_node->children.size());
                    cur_node = cur_node->children[idx].get();
                }
            }
        }
    }

    pasl::pctl::parray<bool> contains(pasl::pctl::parray<T> const& arr) const
    {
        pasl::pctl::raw raw_marker;
        if (root.get() == nullptr)
        {
            return pasl::pctl::parray<bool>(raw_marker, arr.size(), false);
        }
        pasl::pctl::parray<bool> result(raw_marker, arr.size());
        root->do_contains(arr, result, 0, arr.size());
        return result;
    }

    pasl::pctl::parray<bool> insert(pasl::pctl::parray<T> const& arr) const
    {
        pasl::pctl::raw raw_marker;
        if (root.get() == nullptr)
        {
            root = build_from_keys(arr, this->_size_threshold);
            return pasl::pctl::parray<bool>(raw_marker, arr.size(), true);
        }
        pasl::pctl::parray<bool> result = this->contains(arr); // TODO: try non-batch contains
        assert(result.size() == arr.size());

        pasl::pctl::parallel_for(
            static_cast<uint64_t>(0), static_cast<uint64_t>(result.size()),
            [&result](uint64_t idx)
            {
                result[idx] = !result[idx];
            } 
        );
        pasl::pctl::parray<T> insert_keys = pasl::pctl::filteri(
            arr.begin(), arr.end(),
            [&result](uint64_t idx, T const&)
            {
                return result[idx];
            }
        );
        
        if (insert_keys.size() > 0)
        {
            auto insert_res = this->do_instert(insert_keys, this->_size_threshold, 0, insert_keys.size());
            if (insert_res.has_value())
            {
                root = std::move(insert_res.value());
            }
        }
        
        return result;
    }
private:
    std::unique_ptr<ist_internal_node<T>> root;
    const uint64_t _size_threshold;
};
