#pragma once
#include <volk/volk.h>

#include "Buffer.h"

namespace Helios
{
	class IndexBuffer
	{
	public:
		static Ref<IndexBuffer> create(void* data, size_t size, size_t count)
		{
			Ref<IndexBuffer> ib = make_ref<IndexBuffer>();
            ib->init(data, size, count);
			return ib;
		}

		/**
		 * \brief create a new IndexBuffer.
		 * \param data The indices.
		 * \param size The size (in bytes) of the indices.
		 * \param count The index count.
		 */
		static Unique<IndexBuffer> create_unique(void* data, size_t size, size_t count)
		{
			Unique<IndexBuffer> ib = make_unique<IndexBuffer>();
            ib->init(data, size, count);
			return ib;
		}


		const VkBuffer& get_vk_buffer() const { return m_buffer->get_vk_buffer(); }

		uint32_t get_index_count() const { return static_cast<uint32_t>(m_index_count); }

		IndexBuffer() = default;
		~IndexBuffer() = default;

		IndexBuffer(const IndexBuffer&) = delete;
		IndexBuffer& operator=(const IndexBuffer&) = delete;
		IndexBuffer(IndexBuffer&&) = delete;
		IndexBuffer& operator=(IndexBuffer&&) = delete;

	private:
		void init(void* data, size_t size, size_t count);

	private:
		Unique<Buffer> m_buffer;
		size_t m_index_count;
	};
}
