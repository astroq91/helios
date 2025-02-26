#pragma once

#include <volk/volk.h>

#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "Helios/Assets/Asset.h"
#include "Helios/Core/Core.h"

namespace Helios
{
	class Geometry : public Resource, public Asset
	{
	public:
		static Ref<Geometry> create(const std::string& file)
		{
			Ref<Geometry> obj = make_ref<Geometry>();
            obj->init_uuid();
            obj->init_asset(file);
            obj->init(file);
			return obj;
		}

		static Ref<Geometry> create(const std::string& name, void* vertices, size_t vertices_size, void* indices,
                                size_t indices_size,
                                size_t indices_count)
		{
			Ref<Geometry> obj = make_ref<Geometry>();
            obj->init_uuid();
            obj->init_asset(name);
            obj->init(vertices, vertices_size, indices, indices_size, indices_count);
			return obj;
		}

		const Ref<VertexBuffer>& get_vertex_buffer() const { return m_vertex_buffer; }
		const Ref<IndexBuffer>& get_index_buffer() const { return m_index_buffer; }

		Geometry() = default;
		~Geometry() = default;

		Geometry(const Geometry&) = delete;
		Geometry& operator=(const Geometry&) = delete;
		Geometry(Geometry&&) = delete;
		Geometry& operator=(Geometry&&) = delete;

	private:
		void init(const std::string& file);
		void init(void* vertices, size_t vertices_size, void* indices, size_t indices_size,
                  size_t indices_count);

	private:
		Ref<VertexBuffer> m_vertex_buffer;
		Ref<IndexBuffer> m_index_buffer;
	};
}
