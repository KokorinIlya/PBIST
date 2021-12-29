cmake_minimum_required(VERSION 3.10)
project(PBIST)
set(CMAKE_CXX_STANDARD 17)

link_directories(./lib)

include_directories(./include)
include_directories(./pctl/include)
include_directories(./cmdline/include)
include_directories(./chunkedseq/include)

include_directories(./src/include)

add_compile_options(-fcilkplus -DUSE_CILK_PLUS_RUNTIME -std=c++17)

add_executable(bench_build.out benchmarks/bench_build_manual.cpp)
target_link_libraries(bench_build.out pthread cilkrts)

add_subdirectory(tests)
add_subdirectory(benchmarks)