#ifndef IST_INTERNAL_NODE_H
#define IST_INTERNAL_NODE_H

#include "parray.hpp"
#include <memory>
#include <utility>
#include <cassert>
#include "datapar.hpp"
#include <vector>
#include <functional>

template <typename T>
struct ist_internal_node
{
private:
    // representative keys: consists of pairs {key, exists}
    pasl::pctl::parray<std::pair<T, bool>> keys;
    // children.size() == 0 || children.size() == keys.size() + 1
    pasl::pctl::parray<std::unique_ptr<ist_internal_node<T>>> children;

    const uint64_t initial_size;
    uint64_t cur_size;

    bool all_keys_exist()
    {
        pasl::pctl::parray<std::pair<T, bool>> const& this_keys = this->keys;
        pasl::pctl::parray< bool> exists(
            this_keys.size(), 
            [&this_keys] (long i)
            {
                return this_keys[i].second;
            }
        );
        return pasl::pctl::reduce(
            exists.begin(), exists.end(), true,
            [](bool a, bool b)
            {
                return a && b;
            }
        );
    }

    using node_holder = std::vector<std::pair<T, bool>>;

    using level_holder = std::vector<std::pair<node_holder, bool>>;

    using keys_holder = std::vector<level_holder>;

    void do_dump_keys_by_level_seq(keys_holder& holder, uint64_t level) const
    {
        if (holder.size() < level + 1)
        {
            assert(holder.size() == level);
            level_holder empty;
            holder.push_back(empty);
        }

        node_holder cur_node_holder;
        for (int64_t i = 0; i < keys.size(); ++i)
        {
            auto [cur_key, key_exists] = keys[i];
            cur_node_holder.push_back({cur_key, key_exists});
        }
        holder[level].push_back({cur_node_holder, is_terminal()});

        for (uint64_t i = 0; i < children.size(); ++i)
        {
            children[i]->do_dump_keys_by_level_seq(holder, level + 1);
        }
    }

    void do_dump_keys_seq(std::vector<T>& res) const
    {
        if (is_terminal())
        {
            for (uint64_t i = 0; i < keys.size(); ++i)
            {
                auto [cur_key, exists] = keys[i];
                if (exists)
                {
                    res.push_back(cur_key);
                }
            }
            return;
        }
        for (uint64_t i = 0; i < keys.size(); ++i)
        {
            children[i]->do_dump_keys_seq(res);
            auto [cur_key, exists] = keys[i];
            if (exists)
            {
                res.push_back(cur_key);
            }
        }
        children[children.size() - 1]->do_dump_keys_seq(res);
    }

    pasl::pctl::parray<uint64_t> get_sizes() const
    {

        pasl::pctl::parray<std::pair<T, bool>> const& this_keys = this->keys;
        pasl::pctl::parray<std::unique_ptr<ist_internal_node<T>>> const& this_children = this->children;

        if (is_terminal())
        {
            assert(children.size() == 0);
            return pasl::pctl::parray<uint64_t>(
                keys.size(),
                [&this_keys](long idx)
                {
                    if (this_keys[idx].second)
                    {
                        return 1;
                    }
                    else
                    {
                        return 0;
                    }
                }
            );
        }
        else
        {
            assert(keys.size() + 1 == children.size());
            return pasl::pctl::parray<uint64_t>(
                pasl::pctl::raw{}, keys.size() + children.size(),
                [&this_keys, &this_children](long idx)
                {
                    uint64_t i = static_cast<uint64_t>(idx) / static_cast<uint64_t>(2);
                    if (idx % 2 == 0)
                    {
                        return this_children[i].cur_size;
                    }
                    else if (this_keys[i].second)
                    {
                        return 1;
                    }
                    else
                    {
                        return 0;
                    }
                }
            );
        }
    }

    pasl::pctl::parray<uint64_t> get_borders() const
    {
        pasl::pctl::parray<uint64_t> sizes = get_sizes();
        return pasl::pctl::scan(
            sizes.begin(), sizes.end(), 0,
            [](uint64_t x, uint64_t y)
            {
                return x + y;
            },
            pasl::pctl::scan_type::forward_exclusive_scan
        );
    }

    void do_get_keys(pasl::pctl::parray<T>& keys_holder, uint64_t left, uint64_t right) const
    {
        pasl::pctl::parray<uint64_t> const& borders = get_borders();

        assert(borders.size() > 0);
        assert(0 <= left && left < right && right <= keys_holder.size());
        assert(borders.size() == keys_holder.size() + children.size());

        pasl::pctl::parray<std::pair<T, bool>> const& this_keys = this->keys;
        pasl::pctl::parray<std::unique_ptr<ist_internal_node<T>>> const& this_children = this->children;

        fork2(
            [&this_keys, &keys_holder, &borders, left]()
            {
                pasl::pctl::parallel_for(
                    0, this_keys.size(),
                    [&this_keys, &keys_holder, &borders, left](uint64_t key_idx)
                    {
                        auto [cur_key, exists] = this_keys[key_idx];
                        if (exists)
                        {
                            uint64_t border_idx = 2 * key_idx + 1;
                            assert(border_idx < borders.size());
                            uint64_t key_pos = left + borders[border_idx];
                            keys_holder[key_pos] = cur_key;
                        }
                        // TODO: add assertions on borders & exists
                    }
                );
            },
            
            [&this_children, &keys_holder, &borders, left, right]()
            {
                pasl::pctl::parallel_for(
                    0, this_children.size(),
                    [&this_children, &keys_holder, &borders, left, right](uint64_t child_idx)
                    {
                        uint64_t border_idx = 2 * child_idx;
                
                        uint64_t cur_left = left + borders[border_idx];
                        uint64_t cur_right = right;
                        assert(border_idx < borders.size());
                        if (border_idx < borders.size() - 1)
                        {
                            cur_right = borders[border_idx + 1];
                        }

                        this_children[child_idx]->do_get_keys(keys_holder, cur_left, cur_right);
                    }
                );
            }
        );
    }

public:
    ist_internal_node(
        pasl::pctl::parray<std::pair<T, bool>>&& _keys,
        pasl::pctl::parray<std::unique_ptr<ist_internal_node<T>>>&& _children
    ) : keys(std::move(_keys)), 
        //children(std::move(_children)),
        children(pasl::pctl::raw{}, 0),
        initial_size(keys.size()),
        cur_size(keys.size())
    {
        children = std::move(_children);
        assert(keys.size() > 0 && "Empty key array passed to the constructor");
        assert(
            children.size() == 0 || 
            children.size() == keys.size() + 1 && "Keys and children count doesn't match"
        );
        assert(all_keys_exist() && "Non-existing key passed to the constructor");
    }

    bool is_terminal() const
    {
        return children.size() == 0;
    }

    // O(n) Span, use for testing only
    std::vector<T> dump_keys_seq() const
    {
        std::vector<T> res;
        do_dump_keys_seq(res);
        return res;
    }

    // O(n) Span, use for testing only
    keys_holder dump_keys_by_level_seq() const
    {
        keys_holder holder;
        do_dump_keys_by_level_seq(holder, 0);
        return holder;
    }

    pasl::pctl::parray<T> get_keys() const
    {
        if (cur_size == 0)
        {
            return pasl::pctl::parray<T>(0);
        }
        pasl::pctl::parray<T> keys(pasl::pctl::raw{}, cur_size);
        do_get_keys(keys, 0, cur_size);
        return keys;
    }
};


#endif
