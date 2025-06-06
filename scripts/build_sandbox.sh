#!/bin/bash

cmake -G Ninja  -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S .. -B ../build
cmake --build ../build --target Sandbox -j 14
