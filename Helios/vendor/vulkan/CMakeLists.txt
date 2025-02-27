cmake_minimum_required(VERSION 3.25)
project(Vulkan)


file(GLOB_RECURSE HPP_FILES CONFIGURE_DEPENDS "include/vulkan/*.hpp")
file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS "include/vulkan/*.h")

add_library(Vulkan STATIC
  ${HEADER_FILES}
  ${HPP_FILES}
)
set_target_properties(Vulkan PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(Vulkan PUBLIC
    "./include"
)
target_link_directories(Vulkan PRIVATE
   "./lib_linux" 
)

add_library(Volk STATIC
  "./include/volk/volk.c"
  "./include/volk/volk.h"
)

target_include_directories(Volk PUBLIC
  "./include"
)


