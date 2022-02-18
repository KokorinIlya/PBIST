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
* `CC=bin/clang CXX=bin/clang++ cmake .`
* `make`
* Use `CC=bin/clang CXX=bin/clang++ cmake -DCMAKE_BUILD_TYPE=Debug .` to build in debug mode (for testing)
* Or `CC=bin/clang CXX=bin/clang++ cmake -DCMAKE_BUILD_TYPE=Release .` to build in release mode (for benchmarking)

## Testing

* Run from the project directory (`/home/ubuntu/PBIST`)
* `CILK_NWORKERS=16 ./tests/run_tests.out --gtest_filter=*` to run all tests
* Use `--gtest_filter=insert*` e.g. to specify tests to run

## Benchmarking

* Run from the project directory (`/home/ubuntu/PBIST`)
* `CILK_NWORKERS=16 ./benchmarks/run_benchmarks.out --benchmark_filter=.*` to run all benchmarks
* Use `--benchmark_filter=bench_sum.*` e.g. to specify benchmarks to run
