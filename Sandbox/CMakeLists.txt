file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS "src/*.h")

add_executable(Sandbox 
    ${SOURCE_FILES} 
    ${HEADER_FILES} 
)

target_link_libraries(Sandbox PRIVATE 
    Helios
    ImGuizmo
)

set(RESOURCES_PATH "${CMAKE_SOURCE_DIR}/Sandbox/resources/")

target_compile_definitions(Sandbox PRIVATE -DRESOURCES_PATH="${RESOURCES_PATH}")
