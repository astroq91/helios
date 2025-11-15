# Dependencies
add_subdirectory("vendor/vulkan")
add_subdirectory("vendor/glfw")
add_subdirectory("vendor/ImGui")
add_subdirectory("vendor/yaml-cpp")
add_subdirectory("vendor/lua-5.4.6")
add_subdirectory("vendor/tinyfiledialogs")
add_subdirectory("vendor/freetype")
add_subdirectory("vendor/JoltPhysics/Build")
 
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
    "vendor/glm/include"
    "vendor/stb_image/include"
    "vendor/entt/include"
    "vendor/tiny_obj_loader/include"
    "vendor/stduuid/include"
    "vendor/sol2/include"
    "vendor/spdlog/include"
)

target_link_libraries(Helios PUBLIC 
    Volk
    glfw
    ImGui
    yaml-cpp
    lua
    Tinyfiledialogs
    freetype
    Jolt

    $<$<PLATFORM_ID:Linux>:uuid>

    # Platform-specific libraries
    $<$<PLATFORM_ID:Linux>:pthread> 
    $<$<PLATFORM_ID:Linux>:dl>     
)

target_compile_definitions(Helios PUBLIC 
    "GLM_FORCE_QUAT_DATA_XYZW"
    "UUID_SYSTEM_GENERATOR"
    "SOL_ALL_SAFETIES_ON"
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
