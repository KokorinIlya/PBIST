# PBIST
Parallel-Batched Interpolation Search Tree

## Installing dependencies
* `sudo apt update`
* `sudo apt -y install cmake`
* `sudo apt install g++`

## Cloning the repository
* Run from home directory (`/home/ubuntu`)
* `git clone git@github.com:KokorinIlya/PBIST.git`

## Installing PCTL
* Run from home directory (`/home/ubuntu`)
* `wget https://raw.githubusercontent.com/deepsea-inria/pctl/master/script/get.sh`
* `chmod +x ./get.sh`
* `./get.sh $(pwd)/PBIST`

## Compiling
* Run from project directory (`/home/ubuntu/PBIST`)
* `g++ -std=c++17 `pctl/script/print-include-directives.sh $(pwd)` -fcilkplus -lcilkrts -DUSE_CILK_PLUS_RUNTIME file.cpp -o file.out`

## Installing OpenCilk

* Run from home directory (`/home/ubuntu/`)
* `git clone -b opencilk/v1.0 https://github.com/OpenCilk/infrastructure`
* `infrastructure/tools/get $(pwd)/opencilk`
* `infrastructure/tools/build $(pwd)/opencilk $(pwd)/build`
* `cd $(pwd)/build`
* `cmake -DCMAKE_INSTALL_PREFIX=/home/ubuntu/PBIST -P cmake_install.cmake`

## Compiling via CMake

* Run from project directory (`/home/ubuntu/parallel-algorithms/cilk/`)
* `CC=bin/clang CXX=bin/clang++ cmake .`
* `make`
