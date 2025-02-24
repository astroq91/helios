#!/bin/bash
set -e

cd ../Helios/vendor/physx
./generate_projects.sh
cd compiler/linux-clang-debug
make -j${nproc}
