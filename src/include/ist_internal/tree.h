#pragma once

#include "node.h"
#include "build.h"
#include <memory>

template <typename T>
struct ist_internal_tree
{
public:
    ist_internal_tree(pasl::pctl::parray<T> const& keys, uint64_t size_threshold)
        : root(build_from_keys(keys, size_threshold))
    {}
private:
    std::unique_ptr<ist_internal_node<T>> root;
};
