cmake_minimum_required(VERSION 3.10)
project(PBIST)
set(CMAKE_CXX_STANDARD 17)

link_directories(./lib)
include_directories(./include)
include_directories(./pctl/include)
include_directories(./cmdline/include)
include_directories(./chunkedseq/include)
include_directories(./src/include)
include_directories(./)

add_compile_options(-fcilkplus -lcilkrts -DUSE_CILK_PLUS_RUNTIME)

add_executable(bench_sum.out benchmarks/bench_sum.cpp)
target_link_libraries(bench_sum.out pthread cilkrts)

add_subdirectory(tests)