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

    using keys_holder = std::vector<
        std::vector<
            std::vector<std::pair<T, bool>>
        >
    >;

    void do_dump_keys(keys_holder& holder, uint32_t level) const
    {
        if (holder.size() < level + 1)
        {
            assert(holder.size() == level);
            std::vector<std::vector<std::pair<T, bool>>> empty;
            holder.push_back(empty);
        }
        std::vector<std::pair<T, bool>> empty;
        holder[level].push_back(empty);
        for (int64_t i = 0; i < keys.size(); ++i)
        {
            auto [cur_key, key_exists] = keys[i];
            holder[level].back().push_back({cur_key, key_exists});
        }
        for (uint64_t i = 0; i < children.size(); ++i)
        {
            children[i]->do_dump_keys(holder, level + 1);
        }
    }

public:
    ist_internal_node(
        pasl::pctl::parray<std::pair<T, bool>>&& _keys,
        pasl::pctl::parray<std::unique_ptr<ist_internal_node<T>>>&& _children
    ) : keys(std::move(_keys)), 
        //children(std::move(_children))
        children(0, [] (long) { return nullptr; } )
    {
        children = std::move(_children);
        assert(keys.size() > 0 && "Empty key array passed to the constructor");
        assert(children.size() == 0 || children.size() == keys.size() + 1 && "Keys and children count doesn't match");
        assert(all_keys_exist() && "Non-existing key passed to the constructor");
    }

    bool is_terminating() const
    {
        return children.is_empty();
    }

    // O(n) Span, use for testing only
    keys_holder dump_keys() const
    {
        keys_holder holder;
        do_dump_keys(holder, 0);
        return holder;
    }
};


#endif
