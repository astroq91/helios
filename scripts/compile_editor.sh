#!/bin/bash

CC=-DCMAKE_C_COMPILER=gcc-13
CXX=-DCMAKE_CXX_COMPILER=g++-13

echo "Building Helios..."
cmake $CC $CXX -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON .. -B ../build
cmake --build ../build --target Editor -j 14
