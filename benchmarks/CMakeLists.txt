project(benchmarks)

add_subdirectory(lib)
include_directories(lib/include)
link_directories(lib/src)

add_executable(run_benchmarks.out 
    all_benchmarks.cpp
)
target_link_libraries(run_benchmarks.out pthread benchmark_main)

# add_executable(count_access.out count_access.cpp)
# target_link_libraries(count_access.out pthread)

# add_executable(bench_sum_ng.out bench_sum_ng.cpp)
# target_link_libraries(bench_sum_ng.out pthread)

# add_executable(bench_sort_ng.out bench_sort_ng.cpp)
# target_link_libraries(bench_sort_ng.out pthread)