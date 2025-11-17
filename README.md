
<!-- ABOUT THE PROJECT -->
# About The Project

Helios is a general purpose 3D game engine written in C++23, for Windows and Linux (Ubuntu 24.04 LTS with X11 tested). It currently has the following features:

* Instance based rendering, using [Vulkan].
* Physics using [JoltPhysics], for simulating rigidbodies (only boxes)
* Scripting using [Lua].
* ECS using [entt].
* Simple project system.
* An editor, written using [ImGui].

<!-- GETTING STARTED -->
# Getting Started

## Prerequisites

Regardless of the platform, the following dependencies need to be installed:

* CMake (min version 3.5).
* Ninja to build.
* glslc (for compiling shaders).
* Vulkan drivers (and preferably validation layers).

For Windows you need:

* VC17 (i.e. Visual Studio 2022).

For Linux you need:

* C++23 compatible compiler.

## Installation

**Windows:**

1. Clone the repo with the submodules

   ```batch
    git clone --recurse-submodules https://gitlab.com/astroq/helios
   ```

2. Compile the shaders (make sure glslc is added to PATH)

    ```batch
    .\compile_shaders.bat
   ```

3. Generate the Visual Studio projects, and build the `Editor` project

    ```batch
    .\generate_projects.bat
    ```

There is also ``delete_projects.bat`` that deletes all generated Visual Studio project files. 
Be careful as it does this recursively.

**Linux:**

1. Clone the repo with the submodules

   ```bash
    git clone --recurse-submodules https://gitlab.com/astroq/helios
   ```

2. Compile the shaders (make sure glslc is added to PATH)

    ```bash
    cd Helios/scripts
    ./compile_shaders.sh
   ```

3. Building and running the editor

    ```bash
    ./run_editor.sh
    ```

# Usage

There are a two examples bundled with the program in the examples folder.
When opening a project, select the folder that contains the project.yaml file.

The editor is built using the docking branch of ImGui, which means that the windows can be docked
according to your own preference. After opening up the 'physics_1' example (examples/physics_1), this is how it could look:

![example-image](/repo_assets/physics_example_image.png)

The editor is quite barebones but has features to build a simple tech demo.

* Use the buttons at the top to start and stop the runtime.
* Use the Components Browser to view and edit the properties of an object.
* Use the Entity Browser to create new entities, or select existing ones.

The editor uses two windows for rendering the scene: Editor and Game. The 'Editor' window renders the scene using a scene camera
that will always be turned on. The 'Game' window instead renders the scene through the first available game camera, which means that you
need to add a camera component to an entity to be able to see. Currently a light source is needed for both cameras.

**Settings**

* Instancing: Deals with options related to instancing (i.e. batching entities with the same mesh into one draw call). Multithreading will kick in after a certain number of instances, which you can set here. You can also set the number of threads to use.

## File formats

**Projects and scenes**

Both projects and scenes are saved on disk using yaml, and the layout can be found in the examples.
I don't have good checks in place if a scene or project file is valid or not, so if something loads weirdly
check that the .yaml files are correct.

**Path handling**

All relative paths used in assets, like scene- and material files, are relative to the project folder (i.e. the folder containing the project.yaml file).
Relative paths are written using forward slashes (/), to make projects easier to work with cross-platform.
The only exception is if you use absolute paths, in which case the preferred separator of the OS is used.

# Current problems and aspirations

The project is very barebones, but I'm planning on fixing some things like:

* Improve the error handling. A lot of things can cause the program to crash.
* The scene camera currently requires a light source to see, which is not ideal.
* Improve the scripting workflow.
* Improve lighting pipeline.
* Tidy up the editor by making it look nicer.
* Optimize the physics backend. Currently the way I handle resources
  can be quite bad for large scenes.
* Add more collider options.

Aside from that I also have other features I want to add such as:

* Sound.
* Other rendering techniques.

<!-- LICENSE -->
# License

Distributed under the MIT license. See `LICENSE.txt` for more information.

# Vendors

* [Vulkan] - A rendering API.
* [GLFW] - A window management library.
* [JoltPhysics] - A physics engine. 
* [Lua] - A simple scripting language.
* [glm] - A math library.
* [ImGui] - An immediate mode GUI library.
* [entt] - A ECS library.
* [sol2] - A library to make lua integration easier.
* [yaml-cpp] - A library used to load and save yaml files.
* [spdlog] - A logging library.
* [stb_image] - A library used to load images.
* [stduuid] - A library used to generate unique ids.
* [tiny_obj_loader] - A library used to load models.
* [tinyfiledialogs] - A library that handles cross-platform dialogs.

[Vulkan]: https://www.vulkan.org/
[GLFW]: https://www.glfw.org/
[ImGui]: https://github.com/ocornut/imgui
[entt]: https://github.com/skypjack/entt
[glm]: https://github.com/g-truc/glm
[JoltPhysics]: https://github.com/jrouwe/JoltPhysics
[Lua]: https://www.lua.org/
[sol2]: https://github.com/ThePhD/sol2
[yaml-cpp]: https://github.com/jbeder/yaml-cpp
[spdlog]: https://github.com/gabime/spdlog
[stb_image]: https://github.com/nothings/stb
[stduuid]: https://github.com/mariusbancila/stduuid
[tiny_obj_loader]: https://github.com/tinyobjloader/tinyobjloader
[tinyfiledialogs]: https://sourceforge.net/projects/tinyfiledialogs/
