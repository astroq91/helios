#!/bin/bash

CC=-DCMAKE_C_COMPILER=/usr/bin/gcc-13
CXX=-DCMAKE_CXX_COMPILER=/usr/bin/g++-13

echo "Building Helios..."
cmake $CC $CXX -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON .. -B ../build
cmake --build ../build -j 14
