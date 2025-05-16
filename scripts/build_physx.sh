#!/bin/bash
set -e

cd ../Helios/vendor/physx
./generate_projects.sh

if [ -d "compiler/linux-gcc-cpu-only-debug" ]; then
  cd "compiler/linux-gcc-cpu-only-debug"
elif [ -d "compiler/linux-clang-cpu-only-debug" ]; then
  cd "compiler/linux-clang-cpu-only-debug"
fi
make -j${nproc}
