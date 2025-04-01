#pragma once

#include <volk/volk.h>

#include "Helios/Assets/Asset.h"
#include "Helios/Core/Core.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include <filesystem>

namespace Helios {
class Mesh : public Resource, public Asset {
  public:
    static SharedPtr<Mesh> create(const std::filesystem::path& path) {
        SharedPtr<Mesh> obj = SharedPtr<Mesh>::create();
        obj->init_uuid();
        obj->init_asset(path.string());
        if (!obj->init(path)) {
            return nullptr;
        }
        return obj;
    }

    static SharedPtr<Mesh> create(const std::string& name, void* vertices,
                                  size_t vertices_size, void* indices,
                                  size_t indices_size, size_t indices_count) {
        SharedPtr<Mesh> obj = SharedPtr<Mesh>::create();
        obj->init_uuid();
        obj->init_asset(name);
        obj->init(vertices, vertices_size, indices, indices_size,
                  indices_count);
        return obj;
    }

    const SharedPtr<VertexBuffer>& get_vertex_buffer() const {
        return m_vertex_buffer;
    }
    const SharedPtr<IndexBuffer>& get_index_buffer() const {
        return m_index_buffer;
    }

    Mesh() = default;
    ~Mesh() = default;

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&&) = delete;
    Mesh& operator=(Mesh&&) = delete;

  private:
    bool init(const std::filesystem::path& file);
    bool init(void* vertices, size_t vertices_size, void* indices,
              size_t indices_size, size_t indices_count);

  private:
    SharedPtr<VertexBuffer> m_vertex_buffer;
    SharedPtr<IndexBuffer> m_index_buffer;
};
} // namespace Helios
