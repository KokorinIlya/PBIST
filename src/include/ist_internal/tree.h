#pragma once

#include "node.h"
#include "build.h"
#include <memory>
#include "search.h"
#include <cassert>
#include "parray.hpp"
#include "datapar.hpp"
#include "utils.h"

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
            assert(cur_node->keys.size() == cur_node->keys_exist.size());
            auto [idx, found] = binary_search(cur_node->keys, search_key);
            if (found)
            {
                assert(cur_node->keys[idx] == search_key);
                return cur_node->keys_exist[idx];
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

    pasl::pctl::parray<bool> contains(pasl::pctl::parray<T> const& keys) const
    {
        assert(is_sorted(keys, true));
        if (keys.size() == 0)
        {
            return pasl::pctl::parray<bool>(0);
        }
        pasl::pctl::raw raw_marker;
        if (root.get() == nullptr)
        {
            return pasl::pctl::parray<bool>(raw_marker, keys.size(), false);
        }
        pasl::pctl::parray<bool> result(raw_marker, keys.size());
        root->do_contains(keys, result, 0, keys.size());
        return result;
    }

    pasl::pctl::parray<bool> insert(pasl::pctl::parray<T> const& keys)
    {
        assert(is_sorted(keys, true));
        if (keys.size() == 0)
        {
            return pasl::pctl::parray<bool>(0);
        }
        pasl::pctl::raw raw_marker;
        if (root.get() == nullptr)
        {
            root = build_from_keys(keys, this->_size_threshold);
            return pasl::pctl::parray<bool>(raw_marker, keys.size(), true);
        }
        /*
        TODO: try non-batch contains & contains with inplace inversion
        */
        pasl::pctl::parray<bool> result = this->contains(keys);
        assert(result.size() == keys.size());

        pasl::pctl::parallel_for(
            static_cast<uint64_t>(0), static_cast<uint64_t>(result.size()),
            [&result](uint64_t idx)
            {
                result[idx] = !result[idx];
            } 
        );
        pasl::pctl::parray<T> insert_keys = pasl::pctl::filteri(
            keys.begin(), keys.end(),
            [&result](uint64_t idx, T const&)
            {
                return result[idx];
            }
        );
        
        if (insert_keys.size() > 0)
        {
            auto insert_res = root->do_insert(insert_keys, this->_size_threshold, 0, insert_keys.size());
            if (insert_res.has_value())
            {
                root = std::move(insert_res.value());
            }
        }
        
        return result;
    }

    pasl::pctl::parray<bool> remove(pasl::pctl::parray<T> const& keys)
    {
        assert(is_sorted(keys, true));
        if (keys.size() == 0)
        {
            return pasl::pctl::parray<bool>(0);
        }
        pasl::pctl::raw raw_marker;
        if (root.get() == nullptr)
        {
            return pasl::pctl::parray<bool>(raw_marker, keys.size(), false);
        }
        /*
        TODO: try non-batch contains
        */
        pasl::pctl::parray<bool> result = this->contains(keys);
        assert(result.size() == keys.size());

        pasl::pctl::parray<T> remove_keys = pasl::pctl::filteri(
            keys.begin(), keys.end(),
            [&result](uint64_t idx, T const&)
            {
                return result[idx];
            }
        );
        
        if (remove_keys.size() > 0)
        {
            auto remove_res = root->do_remove(remove_keys, this->_size_threshold, 0, remove_keys.size());
            if (remove_res.has_value())
            {
                root = std::move(remove_res.value());
            }
        }
        
        return result;
    }

    /*
    Must return exactly the contents of root->cur_size (or 0, if root is nullptr), use for testing only
    */
    uint64_t calc_tree_size() const
    {
        if (root.get() == nullptr)
        {
            return 0;
        }
        else
        {
            return root->calc_node_size();
        }
    }

    std::vector<access_stats> get_access_stats() const
    {
        std::vector<access_stats> res;
        if (root.get() != nullptr)
        {
            root->get_access_stats(res);
        }
        
        std::sort(
            res.begin(), res.end(),
            [](access_stats const& a, access_stats const& b)
            {
                return a.keys_count > b.keys_count;
            }
        );
        return res;
    }
private:
    std::unique_ptr<ist_internal_node<T>> root;
    const uint64_t _size_threshold;
};
