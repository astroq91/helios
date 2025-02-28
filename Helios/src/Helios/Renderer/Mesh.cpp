#include "Mesh.h"

#include "Helios/Core/Application.h"
#include "Helios/Core/IOUtils.h"

#include <tiny_obj_loader.h>

#include "Vertex.h"

namespace Helios {
void Mesh::init(const std::string& file) {
    const VulkanContext& context =
        Application::get().get_vulkan_manager()->get_context();

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    // Triangulation enabled by default
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                          IOUtils::resolve_path(
                              Application::get().get_asset_base_path(), file)
                              .c_str())) {
        HL_ERROR("Failed to load model: {0}\nTinyObj info: {1}", file,
                 warn + err);
        return;
    }

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    std::unordered_map<Vertex, uint32_t> unique_vertices{};

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};

            vertex.position = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2],
            };

            vertex.tex_coords = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                attrib.texcoords[2 * index.texcoord_index + 1],
            };

            vertex.normal = {
                attrib.normals[3 * index.normal_index + 0],
                attrib.normals[3 * index.normal_index + 1],
                attrib.normals[3 * index.normal_index + 2],
            };

            if (!unique_vertices.contains(vertex)) {
                unique_vertices[vertex] =
                    static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(unique_vertices[vertex]);
        }
    }

    init(vertices.data(), sizeof(Vertex) * vertices.size(), indices.data(),
         sizeof(uint32_t) * indices.size(), indices.size());
}

void Mesh::init(void* vertices, size_t vertices_size, void* indices,
                size_t indices_size, size_t indices_count) {
    m_vertex_buffer = VertexBuffer::create(vertices, vertices_size);
    m_index_buffer = IndexBuffer::create(indices, indices_size, indices_count);
}
} // namespace Helios
