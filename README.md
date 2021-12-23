# PBIST
Parallel-Batched Interpolation Search Tree

## Installing dependencies
* `sudo apt update`
* `sudo apt -y install cmake`
* `sudo apt install g++`

## Cloning the repository
* Run from home directory (`/home/ubuntu`)
* `git clone git@github.com:KokorinIlya/PBIST.git`

## Installing OpenCilk

* Run from home directory (`/home/ubuntu/`)
* `git clone -b opencilk/v1.0 https://github.com/OpenCilk/infrastructure`
* `infrastructure/tools/get $(pwd)/opencilk`
* `infrastructure/tools/build $(pwd)/opencilk $(pwd)/build`
* `cd $(pwd)/build`
* `cmake -DCMAKE_INSTALL_PREFIX=/home/ubuntu/PBIST -P cmake_install.cmake`

## Installing PCTL
* Run from home directory (`/home/ubuntu`)
* `wget https://raw.githubusercontent.com/deepsea-inria/pctl/master/script/get.sh`
* `chmod +x ./get.sh`
* `./get.sh $(pwd)/PBIST`

## Installing Google Tests
* Run from project directory (`/home/ubuntu/PBIST`)
* `git clone https://github.com/google/googletest.git tests/lib`

## Compiling via CMake

* Run from project directory (`/home/ubuntu/PBIST`)
* `CC=bin/clang CXX=bin/clang++ cmake .`
* `make`

## Testing

* Run from project directory (`/home/ubuntu/PBIST`)
* `CILK_NWORKERS=16 ./tests/run_tests.out --gtest_filter=*` to run all tests
* Use `--gtest_filter=insert*` e.g. to specify tests to run