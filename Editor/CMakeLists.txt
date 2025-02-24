cmake_minimum_required(VERSION 3.25)
project(Editor)

add_subdirectory("vendor/ImGuizmo")

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS "src/*.h")

add_executable(Editor 
    ${SOURCE_FILES} 
    ${HEADER_FILES} 
)

target_link_libraries(Editor PRIVATE 
    Helios
    ImGuizmo
)

set(RESOURCES_PATH "${CMAKE_SOURCE_DIR}/Editor/resources/")

target_compile_definitions(Editor PRIVATE -DRESOURCES_PATH="${RESOURCES_PATH}")
