#!/bin/bash

base_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../"

cd $base_dir

cd Helios/resources/shaders
mkdir -p bin

echo "Helios:"
for file in *.{vert,frag}; do
    if [ -f "$file" ]; then
        filename=$(basename "$file")
        echo "Compiling $file..."
        glslc "$file" -o "bin/${filename}"
    fi
done

echo ""

cd $base_dir

cd Editor/resources/shaders
mkdir -p bin

echo "Editor:"
for file in *.{vert,frag}; do
    if [ -f "$file" ]; then
        filename=$(basename "$file")
        echo "Compiling $file..."
        glslc "$file" -o "bin/${filename}"
    fi
done