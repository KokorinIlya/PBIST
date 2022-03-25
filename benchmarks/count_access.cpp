#include <random>
#include "ist_internal/build.h"
#include "ist_internal/node.h"
#include "ist_internal/tree.h"
#include <cstdint>
#include "utils.h"
#include <cassert>
#include <set>
#include <tuple>
#include <algorithm>
#include <iostream>
#include <string>
#include <fstream>

void print_arr(std::string const& name, pasl::pctl::parray<int32_t> const& arr, uint64_t sz)
{
    assert(arr.size() > sz);
    std::cout << name << ": [";
    for (uint64_t i = 0; i < sz; ++i)
    {
        std::cout << " " << arr[i];
    }
    std::cout << "...]" << std::endl;
}

int main() 
{
    uint64_t tree_size = 100'000'000;
    uint64_t batch_size = 10'000'000;
    int32_t keys_from = -1'000'000'000;
    int32_t keys_to = 1'000'000'000;
    uint64_t size_threshold = 3;

    std::default_random_engine generator(0);
    std::uniform_int_distribution<int32_t> elements_distribution(keys_from, keys_to);

    pasl::pctl::parray<int32_t> keys = get_batch(tree_size, generator, elements_distribution);
    pasl::pctl::parray<int32_t> batch = get_batch(batch_size, generator, elements_distribution);
    print_arr("keys", keys, 10);
    print_arr("batch", batch, 10);

    auto tree = ist_internal(keys, size_threshold);
    pasl::pctl::parray<bool> result = tree.contains(batch);

    std::vector<std::tuple<uint64_t, uint64_t, uint64_t, uint64_t>> res = tree.get_access_stats();
    std::ofstream file;
    file.open("stats.txt");
    for (std::tuple<uint64_t, uint64_t, uint64_t, uint64_t> const& t : res)
    {
        auto [sz, exp, bin, total] = t;
        if (bin != 0)
        {
            std::cout << "NON-NULL: " << sz << " " << exp << " " << bin << " " << total << std::endl;
        }
        file << sz << " " << exp << " " << bin << " " << total << std::endl;
    }
    file.close();

    return 0;
}
