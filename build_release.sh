mkdir -p build-release
CC=bin/clang CXX=bin/clang++ cmake -DCMAKE_BUILD_TYPE=Release -S . -B build-release/
cd build-release
make
cd ..