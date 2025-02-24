
<!-- ABOUT THE PROJECT -->
## About The Project

Helios is a general purpose 3D game engine written in C++20. It currently has the following features:

* Instance based rendering, using Vulkan.
* Physics using Physx, for simulating rigidbodies (currently only boxes)
* Scripting using lua.
* ECS using entt.
* Simple project system.
* An editor, written using ImGui.


<!-- GETTING STARTED -->
## Getting Started

Helios is primarily written for linux (Ubuntu) and the build guide will be for linux, but everything should (in theory) compile on Windows as well.
The problem is that I don't have any equivalent bat scripts for compiling the program, shaders etc.

### Prerequisites

In order to successfully compile, the following dependencies need to be installed/downloaded:
* gcc13 and g++13, or any other c++20 capable compiler.
* clang (for PhysX library generation)
* CMake (min version 3.25).
* glfw3.
* glslc (for compiling shaders).
* The Vulkan SDK.

### Installation

1. Clone the repo with the submodules
   ```sh
    git clone --recurse-submodules https://gitlab.com/astroq1/helios
   ```
2. Build the PhysX libraries. This might take some tinkering, as the build process can be a bit weird. 
    The general gist is that you generate the make files for your platform, and the generate the library files
    from those. The following script assumes that you are building using clang. 
   ```sh
    cd Helios/scripts
    ./build_physx.sh
   ```
3. Compiling the shaders (make sure glslc in added to PATH)
    ```sh
    cd Helios/scripts
    ./compile_shaders.sh
   ```
4. Compiling and running the editor
    ```sh
    ./compile_editor.sh
    cd ../build/Editor
    ./Editor
    ```

### Usage

There are a two examples bundled with the program in the examples folder. 
When opening a project, select the folder that contains the project.yaml file.

The editor is built using the docking branch of ImGui, which means that the windows can be docked
according to your own preference. After opening up the 'physics_1' example (examples/physics_1), this is how it could look:

![example-image](/repo_assets/example_image.png)

The editor is quite barebones but has features to build a simple tech demo.
* Use the buttons at the top to start and stop the runtime.
* Use the Components Browser to view and edit the properties of an object.
* Use the Entity Browser to create new entities, or select existing ones.

The editor uses two windows for rendering the scene: Editor and Game. The 'Editor' window renders the scene using a scene camera
that will always be turned on. The 'Game' window instead renders the scene through the first available game camera, which means that you
need to add a camera component to an entity to be able to see.


## Current problems and aspirations

The project is very barebones, as a lot of the time was spent on integrating Vulkan, but I still want to continue working on Helios
to get it to a more usable state. I'm planning on fixing some things like:

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
## License

Distributed under the MIT license. See `LICENSE.txt` for more information.

## Vendors 
 
* [Vulkan] - A rendering API.
* [GLFW] - A window management library.
* [Nvidia PhysX] - A physics engine. 
* [lua] - A simple scripting language.
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
[Nvidia PhysX]: https://github.com/NVIDIA-Omniverse/PhysX
[lua]: https://www.lua.org/
[sol2]: https://github.com/ThePhD/sol2
[yaml-cpp]: https://github.com/jbeder/yaml-cpp
[spdlog]: https://github.com/gabime/spdlog
[stb_image]: https://github.com/nothings/stb
[stduuid]: https://github.com/mariusbancila/stduuid
[tiny_obj_loader]: https://github.com/tinyobjloader/tinyobjloader
[tinyfiledialogs]: https://sourceforge.net/projects/tinyfiledialogs/

