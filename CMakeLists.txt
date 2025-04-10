cmake_minimum_required(VERSION 3.5)


project(Helios)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif()
add_compile_definitions(
	VK_NO_PROTOTYPES
)

add_subdirectory("Helios")
add_subdirectory("Editor")
add_subdirectory("Sandbox")



if (WIN32)
# For Visual Studio folder organization
set_property(TARGET 
	format
	ImGui 
	ImGuizmo 
	lua 
	Tinyfiledialogs 
	Volk 
	yaml-cpp
	yaml-cpp-parse 
	yaml-cpp-read 
	yaml-cpp-sandbox 
	PROPERTY FOLDER "Dependencies"
)
endif()