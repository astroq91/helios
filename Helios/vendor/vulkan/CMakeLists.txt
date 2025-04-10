cmake_minimum_required(VERSION 3.5)
project(Vulkan)

add_library(Volk STATIC
  "./include/volk/volk.c"
  "./include/volk/volk.h"
)

target_include_directories(Volk PUBLIC
  "./include"
)