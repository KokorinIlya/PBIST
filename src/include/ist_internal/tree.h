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
        : root(build_from_keys(keys, size_threshold))
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
                    assert(idx == cur_node->children.size() - 1 || cur_node->children[idx].get() != nullptr);
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
private:
    std::unique_ptr<ist_internal_node<T>> root;
};
