# PBIST
Parallel-Batched Interpolation Search Tree

## Installing dependencies
* `sudo apt update`
* `sudo apt -y install cmake`
* `sudo apt install g++`

## Cloning the repository
* Run from the base directory (`/home/ubuntu`)
* `git clone git@github.com:KokorinIlya/PBIST.git`

## Installing OpenCilk

* Run from the base directory (`/home/ubuntu/`)
* `git clone -b opencilk/v1.0 https://github.com/OpenCilk/infrastructure`
* `infrastructure/tools/get $(pwd)/opencilk`
* `infrastructure/tools/build $(pwd)/opencilk $(pwd)/build`
* `cd build`
* `cmake -DCMAKE_INSTALL_PREFIX=$(pwd)/../PBIST -P cmake_install.cmake`

## Installing PCTL
* Run from the base directory (`/home/ubuntu`)
* `wget https://raw.githubusercontent.com/deepsea-inria/pctl/master/script/get.sh`
* `chmod +x ./get.sh`
* `./get.sh $(pwd)/PBIST`

## Installing Google Tests
* Run from the project directory (`/home/ubuntu/PBIST`)
* `git clone https://github.com/google/googletest.git tests/lib`

## Installing Google Benchmarks
* Run from the project directory (`/home/ubuntu/PBIST`)
* `git clone https://github.com/google/benchmark.git benchmarks/lib`

## Compiling

* Run from the project directory (`/home/ubuntu/PBIST`)
* `chmod +x build_debug.sh` 
* `chmod +x build_release.sh` 
* Use `./build_debug.sh` to build in debug mode (for testing)
* Use `./build_release.sh` to build in release mode (for benchmarking)

## Testing

* Run from the project directory (`/home/ubuntu/PBIST`)
* `CILK_NWORKERS=16 ./build-debug/tests/run_tests.out --gtest_filter=*` to run all tests
* Use `--gtest_filter=insert*` e.g. to specify tests to run

## Benchmarking

* Run from the project directory (`/home/ubuntu/PBIST`)
* `CILK_NWORKERS=16 ./build-release/benchmarks/run_benchmarks.out --benchmark_filter=.*` to run all benchmarks
* Use `--benchmark_filter=bench_sum.*` e.g. to specify benchmarks to run

## Using tcmalloc
* Run from the project directory (`/home/ubuntu/PBIST`)
* `git clone https://github.com/google/tcmalloc.git`
* `vim tcmalloc/tcmalloc/BUILD`
* `bazel test //tcmalloc/...`
* Copy and paste the following text to the end of the file
```
# This library provides standard tcmalloc as a shared (loadable) library.
cc_binary(
    name = "libtcmalloc.so",
    srcs = [
        "libc_override.h",
        "libc_override_gcc_and_weak.h",
        "libc_override_glibc.h",
        "sampler.h",
        "tcmalloc.cc",
        "tcmalloc.h",
    ],
    copts = TCMALLOC_DEFAULT_COPTS,
    visibility = ["//visibility:public"],
    deps = tcmalloc_deps + [
        ":common",
    ],
    linkshared = True,
)
```
* `bazel build //tcmalloc:libtcmalloc.so`
* Set `LD_PRELOAD="tcmalloc/bazel-bin/tcmalloc/libtcmalloc.so"` before executing commands e.g. `LD_PRELOAD="tcmalloc/bazel-bin/tcmalloc/libtcmalloc.so" CILK_NWORKERS=16 ./build-release/benchmarks/run_benchmarks.out --benchmark_filter=bench_tree_build.*`

## Controlling threads affinity
* Use `CILK_NWORKERS=N taskset 0xFFFF numactl --interleave=all command` to execute `command` on cores [0-15], while allocating memory on any NUMA node
* e.g. `CILK_NWORKERS=16  taskset 0xFFFF numactl --interleave=all ./build-release/benchmarks/run_benchmarks.out --benchmark_filter=bench_tree_build.*`
* Use `CILK_NWORKERS=N  taskset 0xFFFF numactl --cpubind=0 --membind=0 command` to execute `command` on cores [0-15], while allocating memory only on `0` NUMA node
* e.g. `CILK_NWORKERS=16  taskset 0xFFFF numactl --cpubind=0 --membind=0 ./build-release/benchmarks/run_benchmarks.out --benchmark_filter=bench_tree_build.*`
