#!/bin/bash

echo "Building Helios..."
cmake -G Ninja  -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S .. -B ../build
cmake --build ../build --target Editor -j 14
