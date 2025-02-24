cmake_minimum_required(VERSION 3.25)
project(Helios)

find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)

# Dependencies
add_subdirectory("vendor/Glad")
add_subdirectory("vendor/ImGui")
add_subdirectory("vendor/yaml-cpp")
add_subdirectory("vendor/lua-5.4.6")
add_subdirectory("vendor/luacpp")
add_subdirectory("vendor/tinyfiledialogs")

option(PHYSX_BUILD_TYPE "The build type of PhysX, i.e., one of {debug, checked, profile, release}" "checked")


if(CMAKE_BUILD_TYPE AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    message("Building snippet in debug configuration")
    set(PHYSX "${CMAKE_SOURCE_DIR}/Helios/vendor/physx/bin/linux.clang/debug")
else()
    message("Building snippet in release configuration with PhysX ${PHYSX_BUILD_TYPE} configuration")
    set(PHYSX "${CMAKE_SOURCE_DIR}/Helios/vendor/physx/bin/linux.clang/debug/${PHYSX_BUILD_TYPE}")
endif()


file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS "src/*.h")

add_library(Helios STATIC
    ${SOURCE_FILES} 
    ${HEADER_FILES} 
)

target_precompile_headers(Helios PRIVATE
    "src/hlpch.h"
)

target_include_directories(Helios PUBLIC
    "src"
    ${Vulkan_INCLUDE_DIRS}
    "vendor/glm/include"
    "vendor/stb_image/include"
    "vendor/entt/include"
    "vendor/tiny_obj_loader/include"
    "vendor/stduuid/include"
    "vendor/sol2/include"
    "vendor/spdlog/include"
    "vendor/physx/include"
)

target_link_libraries(Helios PUBLIC 
    glfw
    ${Vulkan_LIBRARIES} 
    Glad
    ImGui
    yaml-cpp
    lua
    #Luacpp
    Tinyfiledialogs
    $<$<PLATFORM_ID:Linux>:uuid>

    # This is horrible. But this linking order works for my system
    "${PHYSX}/libPhysXExtensions_static_64.a"
    "${PHYSX}/libPhysXCooking_static_64.a"
    "${PHYSX}/libPhysX_static_64.a"
    "${PHYSX}/libPhysXPvdSDK_static_64.a"
    "${PHYSX}/libPhysX_static_64.a"
    "${PHYSX}/libPhysXCharacterKinematic_static_64.a"
    "${PHYSX}/libPhysXCommon_static_64.a"
    "${PHYSX}/libPhysXFoundation_static_64.a"
    "${PHYSX}/libPhysXVehicle2_static_64.a"
    "${PHYSX}/libPhysXVehicle_static_64.a"
    "${PHYSX}/libSnippetRender_static_64.a"
    "${PHYSX}/libSnippetUtils_static_64.a"

    pthread
    dl
)

target_compile_definitions(Helios PUBLIC 
    "GLM_FORCE_QUAT_DATA_XYZW"
    $<$<CONFIG:Debug>:_DEBUG>  
    $<$<CONFIG:Release>:_RELEASE>
    
    $<$<PLATFORM_ID:Linux>:_LINUX>
    $<$<PLATFORM_ID:Windows>:_WINDOWS>
)

set(RESOURCES_PATH "${CMAKE_SOURCE_DIR}/Helios/resources/")
target_compile_definitions(Helios PRIVATE 
  -DRESOURCES_PATH="${RESOURCES_PATH}" 
  -DBASE_DIR="${CMAKE_SOURCE_DIR}/"
)
