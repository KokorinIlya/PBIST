#pragma once

#include "parray.hpp"
#include "psort.hpp"
#include <memory>
#include <utility>
#include <cassert>
#include "datapar.hpp"
#include <vector>
#include <functional>
#include "utils.h"
#include "search.h"
#include <optional>
#include "build.h"

template <typename T>
struct ist_internal;

template <typename T>
struct ist_internal_node
{
private:
    // representative keys
    pasl::pctl::parray<T> keys;
    // keys_exist.size() == keys.size()
    pasl::pctl::parray<bool> keys_exist;
    // children.size() == 0 || children.size() == keys.size() + 1
    pasl::pctl::parray<ist_internal_node<T>*> children;
    pasl::pctl::parray<uint64_t> id;

    uint64_t initial_size;
    uint64_t cur_size;
    uint64_t modifications_count;

    /*
    Structure checking for assertions
    */
    bool is_structure_correct() const
    {
        return keys.size() == keys_exist.size() &&
               keys.size() >= 1 &&
               (
                   children.size() == 0 ||
                   children.size() == keys.size() + 1
               );
    }

    /*
    Insert & delete utils. 
    Coefficient (4) can be adjusted
    */
   bool should_rebuild(uint64_t modifications_count) const
   {
       return 4 * (modifications_count + this->modifications_count) > this->initial_size;
   }

    /*
    Level-by-level keys dumping.
    Has linear span, for testing use only
    */
    using node_holder = std::vector<std::pair<T, bool>>; // <key, exists>

    using level_holder = std::vector<std::pair<node_holder, bool>>; // <node, is_terminal>

    using keys_holder = std::vector<level_holder>;

    void do_dump_keys_by_level_seq(keys_holder& holder, uint64_t level) const
    {
        assert(is_structure_correct());

        if (holder.size() < level + 1)
        {
            assert(holder.size() == level);
            level_holder empty;
            holder.push_back(empty);
        }

        node_holder cur_node_holder;
        for (int64_t i = 0; i < keys.size(); ++i)
        {
            cur_node_holder.push_back({keys[i], keys_exist[i]});
        }
        holder[level].push_back({cur_node_holder, is_terminal()});

        if (!is_terminal())
        {
            assert(keys.size() + 1 == children.size());
            for (uint64_t i = 0; i < children.size(); ++i)
            {
                if (children[i] != nullptr)
                {
                    children[i]->do_dump_keys_by_level_seq(holder, level + 1);
                }
                else
                {
                    if (holder.size() < level + 2)
                    {
                        assert(holder.size() == level + 1);
                        level_holder empty;
                        holder.push_back(empty);
                    }
                    node_holder empty_node_holder;
                    holder[level + 1].push_back({empty_node_holder, true});
                }
            }
        }
        else
        {
            assert(children.size() == 0);
        }
    }

    /*
    Sequential keys dumping.
    Has linear span, for testing use only
    */
    void do_dump_keys_seq(std::vector<T>& res) const
    {
        assert(is_structure_correct());

        if (is_terminal())
        {
            assert(children.size() == 0);
            for (uint64_t i = 0; i < keys.size(); ++i)
            {
                if (keys_exist[i])
                {
                    res.push_back(keys[i]);
                }
            }
            return;
        }
        else
        {
            assert(children.size() == keys.size() + 1);
            for (uint64_t i = 0; i < keys.size(); ++i)
            {
                if (children[i] != nullptr)
                {
                    children[i]->do_dump_keys_seq(res);
                }
                if (keys_exist[i])
                {
                    res.push_back(keys[i]);
                }
            }
            if (children[children.size() - 1] != nullptr)
            {
                children[children.size() - 1]->do_dump_keys_seq(res);
            }
        }
    }

    /*
    Parallel flattening.
    */
    pasl::pctl::parray<uint64_t> get_sizes() const
    {
        assert(is_structure_correct());

        if (is_terminal())
        {
            assert(children.size() == 0);
            return pasl::pctl::parray<uint64_t>(
                keys.size(),
                [this](uint64_t idx)
                {
                    return static_cast<uint64_t>(this->keys_exist[idx]);
                }
            );
        }
        else
        {
            assert(keys.size() + 1 == children.size());
            return pasl::pctl::parray<uint64_t>(
                pasl::pctl::raw{}, keys.size() + children.size(),
                [this](uint64_t idx)
                {
                    uint64_t i = static_cast<uint64_t>(idx) / static_cast<uint64_t>(2);
                    if (idx % 2 == 0)
                    {
                        if (this->children[i] != nullptr)
                        {
                            return this->children[i]->cur_size;
                        }
                        else
                        {
                            return static_cast<uint64_t>(0);
                        }
                    }
                    else
                    {
                        return static_cast<uint64_t>(this->keys_exist[i]);
                    }
                }
            );
        }
    }

    uint64_t get_border_idx_by_key_idx(uint64_t key_idx) const
    {
        assert(is_structure_correct());

        if (is_terminal())
        {
            return key_idx;
        }
        else
        {
            return 2 * key_idx + 1;
        }
    }

    void do_get_keys_from_cur_node(
        pasl::pctl::parray<T>& keys_holder,
        pasl::pctl::parray<uint64_t> const& borders,
        uint64_t left, uint64_t right) const
    {
        assert(is_structure_correct());

        pasl::pctl::parallel_for(
            static_cast<uint64_t>(0), static_cast<uint64_t>(this->keys.size()),
            [this, &keys_holder, &borders, left, right](uint64_t key_idx)
            {
                uint64_t border_idx = this->get_border_idx_by_key_idx(key_idx);
                assert(border_idx < borders.size());
                uint64_t key_pos = left + borders[border_idx];

                uint64_t next_key_pos = right;
                if (border_idx + 1 < borders.size())
                {
                    next_key_pos = left + borders[border_idx + 1];
                }

                if (this->keys_exist[key_idx])
                {
                    assert(next_key_pos == key_pos + 1);
                    keys_holder[key_pos] = this->keys[key_idx];
                }
                else
                {
                    assert(next_key_pos == key_pos);
                }
            }
        );
    }

    void do_get_keys_from_children(
        pasl::pctl::parray<T>& keys_holder,
        pasl::pctl::parray<uint64_t> const& borders,
        uint64_t left, uint64_t right) const
    {
        assert(is_structure_correct());

        pasl::pctl::parallel_for(
            static_cast<uint64_t>(0), static_cast<uint64_t>(this->children.size()),
            [this, &keys_holder, &borders, left, right](uint64_t child_idx)
            {
                uint64_t border_idx = 2 * child_idx;
        
                uint64_t cur_left = left + borders[border_idx];
                uint64_t cur_right = right;
                assert(border_idx < borders.size());
                if (border_idx < borders.size() - 1)
                {
                    cur_right = left + borders[border_idx + 1];
                }

                assert(
                    this->children[child_idx] == nullptr ||
                    this->children[child_idx]->cur_size == cur_right - cur_left
                );
                if (this->children[child_idx] != nullptr && cur_right > cur_left)
                {
                    this->children[child_idx]->do_get_keys(keys_holder, cur_left, cur_right);
                }
            }
        );
    }

    void do_get_keys(pasl::pctl::parray<T>& keys_holder, uint64_t left, uint64_t right) const
    {
        assert(is_structure_correct());
        assert(0 <= left && left < right && right <= keys_holder.size());

        pasl::pctl::parray<uint64_t> sizes = get_sizes();
        assert(sizes.size() == keys.size() + children.size());

        pasl::pctl::parray<uint64_t> borders = pasl::pctl::scan(
            sizes.begin(), sizes.end(), static_cast<uint64_t>(0),
            [](uint64_t x, uint64_t y)
            {
                return x + y;
            },
            pasl::pctl::scan_type::forward_exclusive_scan
        );
        assert(borders.size() == sizes.size());

        if (is_terminal())
        {
            this->do_get_keys_from_cur_node(keys_holder, borders, left, right);
        }
        else
        {
            assert(children.size() == keys.size() + 1);

            pasl::pctl::granularity::cstmt(
                [this]()
                {
                    return this->cur_size;
                },
                [this]()
                {
                    return this->cur_size;
                },
                [this, &keys_holder, &borders, left, right]()
                {
                    pasl::pctl::granularity::fork2(
                        [this, &keys_holder, &borders, left, right]()
                        {
                            this->do_get_keys_from_cur_node(keys_holder, borders, left, right);
                        },
                        [this, &keys_holder, &borders, left, right]()
                        {
                            this->do_get_keys_from_children(keys_holder, borders, left, right);
                        }
                    );
                },
                [this, &keys_holder, &borders, left, right]()
                {
                    this->do_get_keys_from_cur_node(keys_holder, borders, left, right);
                    this->do_get_keys_from_children(keys_holder, borders, left, right);
                }
            );
        }
    }

    /*
    Batch utils
    */
    std::pair<pasl::pctl::parray<uint64_t>, pasl::pctl::parray<uint64_t>> get_range_borders(
        pasl::pctl::parray<int64_t> const& child_idx) const
    {
        assert(is_structure_correct());

        pasl::pctl::raw raw_marker;
        pasl::pctl::parray<uint64_t> all_indexes(
            raw_marker, child_idx.size() + 1,
            [](uint64_t idx)
            {
                return idx;
            }
        );

        /*
        TODO:
        1) two filters may happen in parallel
        2) filter out all -1, then use single filter to obtain borders
        */
        pasl::pctl::parray<uint64_t> range_begins = pasl::pctl::filter(
            all_indexes.begin(), all_indexes.end(),
            [&child_idx](uint64_t cur_idx)
            {
                assert(0 <= cur_idx && cur_idx <= child_idx.size());
                if (cur_idx == 0)
                {
                    return child_idx[0] != -1;
                }
                else if (0 < cur_idx && cur_idx < child_idx.size())
                {
                    return child_idx[cur_idx] != -1 && child_idx[cur_idx] != child_idx[cur_idx - 1];
                }
                else
                {
                    return false;
                }
            }
        );

        pasl::pctl::parray<uint64_t> range_ends = pasl::pctl::filter(
            all_indexes.begin(), all_indexes.end(),
            [&child_idx](uint64_t cur_idx)
            {
                assert(0 <= cur_idx && cur_idx <= child_idx.size());
                if (cur_idx == 0)
                {
                    return false;
                }
                else if (0 < cur_idx && cur_idx < child_idx.size())
                {
                    return child_idx[cur_idx - 1] != -1 && child_idx[cur_idx] != child_idx[cur_idx - 1];
                }
                else
                {
                    return child_idx[child_idx.size() - 1] != -1;
                }
            }
        );

        assert(range_begins.size() == range_ends.size());
        return {range_begins, range_ends};
    }

    std::pair<uint64_t, bool> do_search(T const& key) const
    {
        if (this->id.size() == 0)
        {
            return binary_search(this->keys, key);
        }
        else
        {
            return interpolation_search(this->keys, key, this->id);
        }
    }
     
    /*
    Batch contains
    */
    void non_terminal_do_contains(
       pasl::pctl::parray<T> const& arr, pasl::pctl::parray<bool>& result, 
       uint64_t left_border, uint64_t right_border) const
    {
        assert(is_structure_correct());

        uint64_t range_size = right_border - left_border;
        pasl::pctl::raw raw_marker;
        pasl::pctl::parray<int64_t> child_idx(raw_marker, range_size);

        pasl::pctl::parallel_for(
            left_border, right_border,
            [this, &arr, &result, &child_idx, left_border, right_border](uint64_t key_idx)
            {
                assert(left_border <= key_idx && key_idx < right_border);
                auto [search_idx, found] = do_search(arr[key_idx]);
                if (found)
                {
                    assert(this->keys[search_idx] == arr[key_idx]);
                    result[key_idx] = this->keys_exist[search_idx];
                    child_idx[key_idx - left_border] = -1;
                }
                else
                {
                    child_idx[key_idx - left_border] = search_idx;
                }
            }
        );

        auto range_borders = this->get_range_borders(child_idx);
        pasl::pctl::parray<uint64_t> const& range_begins = range_borders.first;
        pasl::pctl::parray<uint64_t> const& range_ends = range_borders.second;

        // TODO: complexity
        pasl::pctl::parallel_for(
            static_cast<uint64_t>(0), static_cast<uint64_t>(range_begins.size()),
            [this, &range_begins, &range_ends, &arr, &result, &child_idx, left_border, right_border](uint64_t i)
            {
                uint64_t cur_range_begin = range_begins[i];
                uint64_t cur_range_end = range_ends[i];
                assert(
                    0 <= cur_range_begin && 
                    cur_range_begin < cur_range_end && 
                    cur_range_end <= child_idx.size()
                );

                int64_t cur_child_idx = child_idx[cur_range_begin];
                assert(0 <= cur_child_idx && cur_child_idx < this->children.size());

                if (this->children[cur_child_idx] != nullptr)
                {
                    uint64_t next_left_border = left_border + cur_range_begin;
                    uint64_t next_right_border = left_border + cur_range_end;
                    assert(
                        left_border <= next_right_border && 
                        next_left_border < next_right_border && 
                        next_right_border <= right_border
                    );
                    this->children[cur_child_idx]->do_contains(
                        arr, result, next_left_border, next_right_border
                    );
                }
                else
                {
                    pasl::pctl::parallel_for(
                        cur_range_begin, cur_range_end,
                        [&result, left_border](uint64_t cur_key_idx)
                        {
                            result[left_border + cur_key_idx] = false;
                        }
                    );
                }
            }
        );
    }

    /*
    Batch insert
    */
    bool all_keys_not_exist(pasl::pctl::parray<T> const& keys, uint64_t left_border, uint64_t right_border) const
    {
        assert(is_structure_correct());

        pasl::pctl::parray< bool> not_exists(
            right_border - left_border, 
            [this, left_border, &keys] (uint64_t i)
            {
                auto [idx, found] = do_search(keys[left_border + i]);
                assert(
                    !found || 
                    (
                        0 <= idx && idx < this->keys.size() && 
                        this->keys[idx] == keys[left_border + i]
                    )
                );
                return !found || !this->keys_exist[idx];
            }
        );
        return pasl::pctl::reduce(
            not_exists.begin(), not_exists.end(), true,
            [](bool a, bool b)
            {
                return a && b;
            }
        );
    }

    void non_terminal_do_insert(
       pasl::pctl::parray<T> const& keys, uint64_t size_threshold, 
       uint64_t left_border, uint64_t right_border)
    {
        assert(is_structure_correct());

        uint64_t range_size = right_border - left_border;
        pasl::pctl::raw raw_marker;
        pasl::pctl::parray<int64_t> child_idx(raw_marker, range_size);

        pasl::pctl::parallel_for(
            left_border, right_border,
            [this, &keys, &child_idx, left_border, right_border](uint64_t key_idx)
            {
                assert(left_border <= key_idx && key_idx < right_border);
                auto [search_idx, found] = do_search(keys[key_idx]);

                if (found)
                {
                    assert(
                        0 <= search_idx && search_idx < this->keys.size() &&
                        this->keys[search_idx] == keys[key_idx] &&
                        !this->keys_exist[search_idx]
                    );
                    this->keys_exist[search_idx] = true;
                    child_idx[key_idx - left_border] = -1;
                }
                else
                {
                    child_idx[key_idx - left_border] = search_idx;
                }
            }
        );
        
        auto range_borders = this->get_range_borders(child_idx);
        pasl::pctl::parray<uint64_t> const& range_begins = range_borders.first;
        pasl::pctl::parray<uint64_t> const& range_ends = range_borders.second;

        pasl::pctl::parallel_for(
            static_cast<uint64_t>(0), static_cast<uint64_t>(range_begins.size()),
            [
                this, &range_begins, &range_ends, &keys, &child_idx, 
                left_border, right_border, size_threshold
            ](uint64_t i)
            {
                uint64_t cur_range_begin = range_begins[i];
                uint64_t cur_range_end = range_ends[i];
                assert(
                    0 <= cur_range_begin && 
                    cur_range_begin < cur_range_end && 
                    cur_range_end <= child_idx.size()
                );

                int64_t cur_child_idx = child_idx[cur_range_begin];
                assert(0 <= cur_child_idx && cur_child_idx < this->children.size());

                uint64_t next_left_border = left_border + cur_range_begin;
                uint64_t next_right_border = left_border + cur_range_end;
                assert(
                    left_border <= next_right_border && 
                    next_left_border < next_right_border && 
                    next_right_border <= right_border
                );

                if (this->children[cur_child_idx] != nullptr)
                {
                    auto insert_res = this->children[cur_child_idx]->do_insert(
                        keys, size_threshold, next_left_border, next_right_border
                    );
                    if (insert_res.has_value())
                    {
                        delete this->children[cur_child_idx];
                        this->children[cur_child_idx] = std::move(insert_res.value());
                    }
                }
                else
                {
                    this->children[cur_child_idx] = do_build_from_keys(
                        keys, next_left_border, next_right_border, size_threshold
                    );
                }
            }
        );
    }

    /*
    Batch remove
    */
   void non_terminal_do_remove(
       pasl::pctl::parray<T> const& keys, uint64_t size_threshold, 
       uint64_t left_border, uint64_t right_border)
    {
        assert(is_structure_correct());

        uint64_t range_size = right_border - left_border;
        pasl::pctl::raw raw_marker;
        pasl::pctl::parray<int64_t> child_idx(raw_marker, range_size);

        pasl::pctl::parallel_for(
            left_border, right_border,
            [this, &keys, &child_idx, left_border, right_border](uint64_t key_idx)
            {
                assert(left_border <= key_idx && key_idx < right_border);
                auto [search_idx, found] = do_search(keys[key_idx]);

                if (found)
                {
                    assert(
                        0 <= search_idx && search_idx < this->keys.size() &&
                        this->keys[search_idx] == keys[key_idx] &&
                        this->keys_exist[search_idx]
                    );
                    this->keys_exist[search_idx] = false;
                    child_idx[key_idx - left_border] = -1;
                }
                else
                {
                    child_idx[key_idx - left_border] = search_idx;
                }
            }
        );

        auto range_borders = this->get_range_borders(child_idx);
        pasl::pctl::parray<uint64_t> const& range_begins = range_borders.first;
        pasl::pctl::parray<uint64_t> const& range_ends = range_borders.second;

        pasl::pctl::parallel_for(
            static_cast<uint64_t>(0), static_cast<uint64_t>(range_begins.size()),
            [
                this, &range_begins, &range_ends, &keys, &child_idx, 
                left_border, right_border, size_threshold
            ](uint64_t i)
            {
                uint64_t cur_range_begin = range_begins[i];
                uint64_t cur_range_end = range_ends[i];
                assert(
                    0 <= cur_range_begin && 
                    cur_range_begin < cur_range_end && 
                    cur_range_end <= child_idx.size()
                );

                int64_t cur_child_idx = child_idx[cur_range_begin];
                assert(0 <= cur_child_idx && cur_child_idx < this->children.size());

                uint64_t next_left_border = left_border + cur_range_begin;
                uint64_t next_right_border = left_border + cur_range_end;
                assert(
                    left_border <= next_right_border && 
                    next_left_border < next_right_border && 
                    next_right_border <= right_border
                );

                assert(this->children[cur_child_idx] != nullptr);
                auto remove_res = this->children[cur_child_idx]->do_remove(
                    keys, size_threshold, next_left_border, next_right_border
                );
                if (remove_res.has_value())
                {
                    delete this->children[cur_child_idx];
                    this->children[cur_child_idx] = std::move(remove_res.value());
                }
            }
        );
    }

public:
    friend struct ist_internal<T>;

    ist_internal_node(
        pasl::pctl::parray<T>&& _keys,
        pasl::pctl::parray<ist_internal_node<T>*>&& _children,
        uint64_t keys_count
    ) : keys(std::move(_keys)), 
        keys_exist(pasl::pctl::raw{}, keys.size(), true),
        children(std::move(_children)),
        //id(build_id(keys, keys.size())),
        id(pasl::pctl::raw{}, 0),
        initial_size(keys_count),
        cur_size(keys_count),
        modifications_count(static_cast<uint64_t>(0))
    {
        if (cur_size >= 50)
        {
            id = build_id(keys, keys.size());
        }


        assert(keys.size() > 0 && "Empty key array passed to the constructor");
        assert(
            children.size() == 0 || 
            children.size() == keys.size() + 1 && "Keys and children count doesn't match"
        );
    }

    ~ist_internal_node()
    {
        for (uint64_t i = 0; i < this->children.size(); ++i)
        {
            delete this->children[i];
        }
    }

    bool is_terminal() const
    {
        assert(is_structure_correct());
        return children.size() == 0;
    }

    /*
    O(n) Span, use for testing only
    */
    std::vector<T> dump_keys_seq() const
    {
        assert(is_structure_correct());
        std::vector<T> res;
        do_dump_keys_seq(res);
        return res;
    }

    /*
    O(n) Span, use for testing only
    */
    keys_holder dump_keys_by_level_seq() const
    {
        assert(is_structure_correct());
        keys_holder holder;
        do_dump_keys_by_level_seq(holder, 0);
        return holder;
    }

    /*
    Must return exactly the contents of this->cur_size, use for testing only
    */
    uint64_t calc_node_size() const
    {
        assert(is_structure_correct());
        
        pasl::pctl::parray<uint64_t> keys_exist(
            this->keys.size(),
            [this](uint64_t idx)
            {
                return static_cast<uint64_t>(this->keys_exist[idx]);
            }
        );
        uint64_t this_keys_count = pasl::pctl::reduce(
            keys_exist.begin(), keys_exist.end(), static_cast<uint64_t>(0),
            [](uint64_t a, uint64_t b)
            {
                return a + b;
            }
        );

        pasl::pctl::parray<uint64_t> subtree_sizes(
            this->children.size(),
            [this](uint64_t idx)
            {
                if (this->children[idx] == nullptr)
                {
                    return static_cast<uint64_t>(0);
                }
                else
                {
                    return this->children[idx]->calc_node_size();
                }
            }
        );
        uint64_t total_size = pasl::pctl::reduce(
            subtree_sizes.begin(), subtree_sizes.end(), static_cast<uint64_t>(0),
            [](uint64_t a, uint64_t b)
            {
                return a + b;
            }
        ) + this_keys_count;

        assert(total_size == this->cur_size);
        return total_size;
    }

    /*
    Parallel key flatteing with polylog span
    */
    pasl::pctl::parray<T> get_keys() const
    {
        assert(is_structure_correct());
        if (cur_size == 0)
        {
            return pasl::pctl::parray<T>(0);
        }
        pasl::pctl::parray<T> keys(pasl::pctl::raw{}, cur_size);
        do_get_keys(keys, 0, cur_size);
        return keys;
    }

    void do_contains(pasl::pctl::parray<T> const& arr, pasl::pctl::parray<bool>& result, 
                     uint64_t left_border, uint64_t right_border) const
    {
        assert(is_structure_correct());
        assert(arr.size() == result.size());
        assert(0 <= left_border < right_border <= arr.size());

        if (is_terminal())
        {
            pasl::pctl::parallel_for(
                left_border, right_border,
                [this, &arr, &result, left_border, right_border](long key_idx)
                {
                    assert(left_border <= key_idx && key_idx < right_border);
                    auto [search_idx, found] = do_search(arr[key_idx]);
                    if (found)
                    {
                        assert(this->keys[search_idx] == arr[key_idx]);
                        result[key_idx] = this->keys_exist[search_idx];
                    }
                    else
                    {
                        result[key_idx] = false;
                    }
                }
            );
        }
        else
        {
            non_terminal_do_contains(arr, result, left_border, right_border);
        }
    }

    std::optional<ist_internal_node<T>*> do_insert(
        pasl::pctl::parray<T> const& keys, uint64_t size_threshold,
        uint64_t left_border, uint64_t right_border)
    {
        assert(is_structure_correct());
        assert(0 <= left_border < right_border <= keys.size());

        pasl::pctl::raw raw_marker;
        uint64_t new_keys_count = right_border - left_border;

        if (is_terminal() || should_rebuild(new_keys_count))
        {
            /*
            all_keys_not_exist assertion is guaranteed not to return false-positive result
            only for terminal nodes (since inserted keys can exist in children of non-terminal node). 
            However, it's still better, than nothing.
            */
            assert(all_keys_not_exist(keys, left_border, right_border));

            pasl::pctl::parray<T> cur_keys = get_keys();
            pasl::pctl::parray<T> all_keys(raw_marker, cur_keys.size() + new_keys_count);
            pasl::pctl::merge(
                cur_keys.begin(), cur_keys.end(),
                keys.begin() + left_border, keys.begin() + right_border,
                all_keys.begin(),
                std::less<T>{}
            );
            assert(is_sorted(all_keys, true));
            return build_from_keys(all_keys, size_threshold);
        }
        else
        {
            this->modifications_count += new_keys_count;
            this->cur_size += new_keys_count;
            non_terminal_do_insert(keys, size_threshold, left_border, right_border);
            return {};
        }
    }

    std::optional<ist_internal_node<T>*> do_remove(
        pasl::pctl::parray<T> const& keys, uint64_t size_threshold,
        uint64_t left_border, uint64_t right_border)
    {
        assert(is_structure_correct());
        assert(0 <= left_border < right_border <= keys.size());

        pasl::pctl::raw raw_marker;
        uint64_t keys_count = right_border - left_border;
        assert(this->cur_size >= keys_count);
        this->modifications_count += keys_count;
        this->cur_size -= keys_count;

        if (is_terminal())
        {
            pasl::pctl::parallel_for(
                left_border, right_border,
                [this, &keys](uint64_t remove_key_idx)
                {
                    auto [idx, found] = do_search(keys[remove_key_idx]);
                    assert(
                        found && 0 <= idx && idx < this->keys.size() && 
                        this->keys[idx] == keys[remove_key_idx] && 
                        this->keys_exist[idx]
                    );
                    this->keys_exist[idx] = false;
                }
            );
        }
        else
        {
            non_terminal_do_remove(keys, size_threshold, left_border, right_border);
        }

        /*
        TODO: implement parallel setminus on sorted arrays and perform rebuilding before
        removing keys
        */
        if (should_rebuild(0))
        {
            pasl::pctl::parray<T> cur_keys = get_keys();
            assert(is_sorted(cur_keys, true));
            return build_from_keys(cur_keys, size_threshold);
        }
        else
        {
            return {};
        }
    }
};
