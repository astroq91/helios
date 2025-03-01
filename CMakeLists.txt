cmake_minimum_required(VERSION 3.25)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL" CACHE STRING "" FORCE)

project(Helios)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(
	VK_NO_PROTOTYPES
)

add_subdirectory("Helios")
add_subdirectory("Editor")
add_subdirectory("Sandbox")

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
