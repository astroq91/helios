#!/bin/bash
# Compiles shaders recursively for the given directories.

shopt -s globstar nullglob

base_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../"

WHITE='\033[1;37m'
GREEN='\033[0;32m'
NC='\033[0m'

function compile_shaders {
  cd "$base_dir"
  current_dir=""
  for file in "$1"/**/*.{vert,frag}; do
      if [ -f "$file" ]; then
          # Get the directory the file is in
          dir=$(dirname "$file")

          # Print the directory (if new)
          if [ "$dir" != "$current_dir" ]; then
            printf "\n[${WHITE}$dir${NC}]\n"
            current_dir="$dir"
          fi

          # Make a bin/ version of that directory
          out_dir="$dir/bin"
          mkdir -p "$out_dir"

          # Compile to bin/dir/filename.spv
          filename=$(basename "$file")
          printf "Compiling $filename -> bin/${filename}.spv\n"
          glslc "$file" -o "$out_dir/${filename}.spv"
      fi
  done
}

compile_shaders "Helios/resources/shaders"
compile_shaders "Editor/resources/shaders"
compile_shaders "examples"
