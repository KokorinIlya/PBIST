#include <gtest/gtest.h>
#include "ist_internal/node.h"
#include "ist_internal/build.h"
#include <memory>
#include <cstdint>
#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>


void handler(int sig) 
{
    void *array[10];
    size_t size;
    size = backtrace(array, 10);
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
}

TEST(tree_building, simple)
{
    //signal(SIGSEGV, handler);
    pasl::pctl::parray<int32_t> keys(
        10,
        [](long i)
        {
            return static_cast<int32_t>(i);
        }
    );
    std::unique_ptr<ist_internal_node<int32_t>> result = build_from_keys(keys, 3);

    std::cout << "Keys by level:" << std::endl;
    auto stored_keys = result->dump_keys_by_level_seq();
    for (uint64_t i = 0; i < stored_keys.size(); ++i)
    {
        std::cout << "level#" << i << std::endl;
        for (uint64_t j = 0; j < stored_keys[i].size(); ++j)
        {
            std::cout << "node#" << j << std::endl;
            std::cout << "[ ";
            for (auto k : stored_keys[i][j].first)
            {
                std::cout << k.first << ": " << k.second << "; ";
            }
            std::cout << "], terminal = " << stored_keys[i][j].second << std::endl;
        }
    }

    std::cout << "Flattened keys:" << std::endl;
    auto flattened_keys = result->dump_keys_seq();
    std::cout << "[";
    for (uint64_t i = 0; i < flattened_keys.size(); ++i)
    {
        std::cout << flattened_keys[i] << ", ";
    }
    std::cout << "]" << std::endl;
}