project(benchmarks)

add_subdirectory(lib)
include_directories(lib/include)
link_directories(lib/src)

add_executable(run_benchmarks.out 
    all_benchmarks.cpp
)
target_link_libraries(run_benchmarks.out pthread benchmark_main)
