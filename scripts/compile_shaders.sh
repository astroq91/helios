#!/bin/bash

function compile_shaders {
	for file in *.{vert,frag}; do
	    if [ -f "$file" ]; then
		filename=$(basename "$file")
		printf "Compiling $file...\n"
		glslc "$file" -o "bin/${filename}"
	    fi
	done
}

base_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../"
function new_target {
	printf "___________________\n"
	cd $base_dir
	printf "[$1]\n"
}

cd $base_dir

new_target "Helios"
cd Helios/resources/shaders
mkdir -p bin
compile_shaders

new_target "Editor"
cd Editor/resources/shaders
mkdir -p bin

compile_shaders

printf "___________________\n\n"
printf "     Examples\n"

new_target "example1"
cd examples/example1/shaders
mkdir -p bin
compile_shaders

new_target "shader_collection"
cd examples/shader_collection/shaders
mkdir -p bin
compile_shaders

new_target "shader_collection - raymarching"
cd examples/shader_collection/shaders/raymarching
mkdir -p bin
compile_shaders
