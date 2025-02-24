cmake_minimum_required(VERSION 3.20)
project(Glad)

set(SOURCES
    "src/glad.c"
)

set(HEADERS
    "include/glad/glad.h"
    "include/KHR/khrplatform.h"
)

add_library(Glad STATIC 
    ${SOURCES} 
    ${HEADERS} 
)

target_include_directories(Glad PUBLIC
    "include"
)
