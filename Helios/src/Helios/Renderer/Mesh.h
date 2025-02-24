#pragma once

#include <vulkan/vulkan.h>

#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "Helios/Assets/Asset.h"
#include "Helios/Core/Core.h"

namespace Helios
{
	class Mesh : public Resource, public Asset
	{
	public:
		static Ref<Mesh> create(const std::string& file)
		{
			Ref<Mesh> obj = make_ref<Mesh>();
            obj->init_uuid();
            obj->init_asset(file);
            obj->init(file);
			return obj;
		}

		static Ref<Mesh> create(const std::string& name, void* vertices, size_t vertices_size, void* indices,
                                size_t indices_size,
                                size_t indices_count)
		{
			Ref<Mesh> obj = make_ref<Mesh>();
            obj->init_uuid();
            obj->init_asset(name);
            obj->init(vertices, vertices_size, indices, indices_size, indices_count);
			return obj;
		}

		const Ref<VertexBuffer>& get_vertex_buffer() const { return m_vertex_buffer; }
		const Ref<IndexBuffer>& get_index_buffer() const { return m_index_buffer; }

		Mesh() = default;
		~Mesh() = default;

		Mesh(const Mesh&) = delete;
		Mesh& operator=(const Mesh&) = delete;
		Mesh(Mesh&&) = delete;
		Mesh& operator=(Mesh&&) = delete;

	private:
		void init(const std::string& file);
		void init(void* vertices, size_t vertices_size, void* indices, size_t indices_size,
                  size_t indices_count);

	private:
		Ref<VertexBuffer> m_vertex_buffer;
		Ref<IndexBuffer> m_index_buffer;
	};
}
