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


void handler(int sig) 
{
    void *array[10];
    size_t size;

    // get void*'s for all entries on the stack
    size = backtrace(array, 10);

    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
}

TEST(tree_building, simple)
{
    signal(SIGSEGV, handler);
    pasl::pctl::parray<int32_t> keys(
        100,
        [](long i)
        {
            return static_cast<int32_t>(i);
        }
    );
    std::unique_ptr<ist_internal_node<int32_t>> result = build_from_keys(keys, 5);
}