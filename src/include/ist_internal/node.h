#ifndef IST_INTERNAL_NODE_H
#define IST_INTERNAL_NODE_H

#include "parray.hpp"
#include <memory>
#include <utility>
#include <cassert>
#include "datapar.hpp"

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
        return pasl::pctl::reduce(
            keys.begin(), keys.end(), true, 
            [](bool x, bool y)
            {
                return x && y;
            }
        )
    }

public:
    ist_internal_node(
        pasl::pctl::parray<std::pair<T, bool>> && _keys,
        pasl::pctl::parray<std::unique_ptr<ist_internal_node<T>>>&& _children
    ) : keys(_keys), children(_children)
    {
        assert(all_keys_exist() && "Non-existing key passed to the constructor");
    }

    bool is_terminating() const
    {
        return children.is_empty();
    }
};


#endif
