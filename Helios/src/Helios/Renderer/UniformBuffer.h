#pragma once
#include "Buffer.h"

namespace Helios
{
	class UniformBuffer
	{
	public:
		static Ref<UniformBuffer> create(size_t size)
		{
			Ref<UniformBuffer> obj = make_ref<UniformBuffer>();
            obj->init(size);
			return obj;
		}

		static Unique<UniformBuffer> create_unique(size_t size)
		{
			Unique<UniformBuffer> obj = make_unique<UniformBuffer>();
            obj->init(size);
			return obj;
		}

		const Ref<Buffer>& get_buffer() { return m_buffer; }

		void* get_mapped_data() const { return m_mapped_data; }

		UniformBuffer() = default;
		~UniformBuffer() = default;

		UniformBuffer(const UniformBuffer&) = delete;
		UniformBuffer& operator=(const UniformBuffer&) = delete;
		UniformBuffer(UniformBuffer&&) = delete;
		UniformBuffer& operator=(UniformBuffer&&) = delete;

	private:
		void init(size_t size);

	private:
		Ref<Buffer> m_buffer;
		void* m_mapped_data;
	};
}
