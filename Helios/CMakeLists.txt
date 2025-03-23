# Dependencies
add_subdirectory("vendor/vulkan")
add_subdirectory("vendor/glfw")
add_subdirectory("vendor/ImGui")
add_subdirectory("vendor/yaml-cpp")
add_subdirectory("vendor/lua-5.4.6")
add_subdirectory("vendor/tinyfiledialogs")
add_subdirectory("vendor/freetype")
 
# Find the correct path for physx libraries
if (WIN32)
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vendor/physx/bin/win.x86_64.vc143.md/debug")
        set(PHYSX "${CMAKE_CURRENT_SOURCE_DIR}/vendor/physx/bin/win.x86_64.vc143.md/debug")
    elseif (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vendor/physx/bin/win.x86_64.vc142.md/debug")
        set(PHYSX "${CMAKE_CURRENT_SOURCE_DIR}/vendor/physx/bin/win.x86_64.vc142.md/debug")
    else()
        message(FATAL_ERROR "Could not find (MDd static debug) PhysX libraries for vc17 or vc16. Change this if you are using a different compiler.")
    endif()

    set(PHYSX_LIBRARIES
        "${PHYSX}/PhysXExtensions_static_64.lib"
        "${PHYSX}/PhysXCooking_static_64.lib"
        "${PHYSX}/PhysX_static_64.lib"
        "${PHYSX}/PhysXPvdSDK_static_64.lib"
        "${PHYSX}/PhysXCharacterKinematic_static_64.lib"
        "${PHYSX}/PhysXCommon_static_64.lib"
        "${PHYSX}/PhysXFoundation_static_64.lib"
        "${PHYSX}/PhysXVehicle2_static_64.lib"
        "${PHYSX}/PhysXVehicle_static_64.lib"
        "${PHYSX}/SnippetRender_static_64.lib"
        "${PHYSX}/SnippetUtils_static_64.lib"
    )


elseif (UNIX AND NOT APPLE)
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vendor/physx/bin/linux.x86_64/debug")
        set(PHYSX "${CMAKE_CURRENT_SOURCE_DIR}/vendor/physx/bin/linux.x86_64/debug")
    else()
        message(FATAL_ERROR "Could not find (debug) PhysX libraries. Change this if you are using a different compiler.")
    endif()


    # This is horrible. But this linking order works for my system
    set(PHYSX_LIBRARIES
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
    )
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
    Volk
    glfw
    ImGui
    yaml-cpp
    lua
    Tinyfiledialogs
    freetype

    $<$<PLATFORM_ID:Linux>:uuid>

    ${PHYSX_LIBRARIES}

    # Platform-specific libraries
    $<$<PLATFORM_ID:Linux>:pthread> 
    $<$<PLATFORM_ID:Linux>:dl>     
)

target_compile_definitions(Helios PUBLIC 
    "GLM_FORCE_QUAT_DATA_XYZW"
    "UUID_SYSTEM_GENERATOR"
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
