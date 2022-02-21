cmake_minimum_required(VERSION 3.10)
project(PBIST)
set(CMAKE_CXX_STANDARD 17)
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -fopencilk")

link_directories(./lib)

include_directories(./include)
include_directories(./pctl/include)
include_directories(./cmdline/include)
include_directories(./chunkedseq/include)
include_directories(./src/include)

add_link_options(-fopencilk)
add_compile_options(-fopencilk -DUSE_CILK_PLUS_RUNTIME -std=c++17 -O2)

add_subdirectory(tests)
add_subdirectory(benchmarks)
