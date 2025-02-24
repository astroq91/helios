cmake_minimum_required(VERSION 3.20)
project(Tinyfiledialogs)

set(SOURCES
    "tinyfiledialogs.c"
)

set(HEADERS
    "tinyfiledialogs.h"
)

add_library(Tinyfiledialogs STATIC 
    ${SOURCES} 
    ${HEADERS} 
)

target_include_directories(Tinyfiledialogs PUBLIC
    "."
)
