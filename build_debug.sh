mkdir -p build-debug
CC=bin/clang CXX=bin/clang++ cmake -DCMAKE_BUILD_TYPE=Debug -S . -B build-debug/
cd build-debug
make
cd ..