@echo off
set CC=cl
set CXX=cl
set CXXFLAGS=/std:c++20

echo "Building Editor..."

cmake -DCMAKE_C_COMPILER=%CC% -DCMAKE_CXX_COMPILER=%CXX% -DCMAKE_CXX_FLAGS=%CXXFLAGS% -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON .. -B ..\build

cmake --build ..\build --config Debug --parallel 14