#pragma once

#include <volk/volk.h>

#include "Buffer.h"

namespace Helios
{
	class VertexBuffer
	{
	public:
		/**
		 * \brief create a new VertexBuffer.
		 * \param data The vertices.
		 * \param size The size (in bytes) of the vertices.
		 */
		static SharedPtr<VertexBuffer> create(void* data, size_t size)
		{
			SharedPtr<VertexBuffer> vb = SharedPtr<VertexBuffer>::create();
            vb->init(data, size);
			return vb;
		}

		void insert_memory(void* data, size_t size, uint32_t offset = 0);

		const VkBuffer& get_vk_buffer() const { return m_buffer->get_vk_buffer(); }

		VertexBuffer() = default;
		~VertexBuffer() = default;

		VertexBuffer(const VertexBuffer&) = delete;
		VertexBuffer& operator=(const VertexBuffer&) = delete;
		VertexBuffer(VertexBuffer&&) = delete;
		VertexBuffer& operator=(VertexBuffer&&) = delete;

	private:
		void init(void* data, size_t size);

	private:
		std::unique_ptr<Buffer> m_buffer;
	};
}
