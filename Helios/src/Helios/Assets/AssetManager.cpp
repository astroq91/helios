#include "AssetManager.h"

#include "Helios/Core/Log.h"
#include "Helios/Core/SharedPtr.h"
#include "Helios/Renderer/MeshVertex.h"

namespace {
using namespace Helios;

std::vector<MeshVertex> cube_vertices = {
    // Front face
    {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
    // Back face
    {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
    {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
    {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},
    // Top face
    {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
    // Bottom face
    {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
    {{-0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},
    // Left face
    {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{-0.5f, -0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
    {{-0.5f, 0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
    // Right face
    {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
    {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
};

std::vector<uint32_t> cube_indices = {
    // Front face
    0, 2, 3, 2, 0, 1,
    // Back face
    4, 6, 7, 6, 4, 5,
    // Top face
    8, 10, 11, 10, 8, 9,
    // Bottom face
    12, 14, 15, 14, 12, 13,
    // Left face
    16, 18, 19, 18, 16, 17,
    // Right face
    20, 22, 23, 22, 20, 21};

std::vector<MeshVertex> quad_vertices = {
    // Front face
    {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
};
std::vector<uint32_t> quad_indices = {
    0, 2, 3, 2, 0, 1,
};
} // namespace

namespace Helios {
void AssetManager::init() {
    /* Create meshes */
    add_mesh(Mesh::create(
        "Cube", cube_vertices.data(), sizeof(MeshVertex) * cube_vertices.size(),
        cube_indices.data(), sizeof(uint32_t) * cube_indices.size(),
        cube_indices.size()));
    add_mesh(Mesh::create(
        "Quad", quad_vertices.data(), sizeof(MeshVertex) * quad_vertices.size(),
        quad_indices.data(), sizeof(uint32_t) * quad_indices.size(),
        quad_indices.size()));

    add_texture(Texture::create(
        {
            .right = RESOURCES_PATH "images/default_skybox/right.jpg",
            .left = RESOURCES_PATH "images/default_skybox/left.jpg",
            .top = RESOURCES_PATH "images/default_skybox/top.jpg",
            .bottom = RESOURCES_PATH "images/default_skybox/bottom.jpg",
            .front = RESOURCES_PATH "images/default_skybox/front.jpg",
            .back = RESOURCES_PATH "images/default_skybox/back.jpg",
        },
        "default_skybox"));
}

void AssetManager::clear_assets() {
    m_meshes.clear();
    m_shaders.clear();
    m_textures.clear();
    m_materials.clear();

    // Recreate the default assets
    init();
}
} // namespace Helios
