#pragma once
#include "Buffer.h"

namespace Helios
{
	class UniformBuffer
	{
	public:
		static SharedPtr<UniformBuffer> create(size_t size)
		{
			SharedPtr<UniformBuffer> obj = SharedPtr<UniformBuffer>::create();
            obj->init(size);
			return obj;
		}

		static std::unique_ptr<UniformBuffer> create_unique(size_t size)
		{
			std::unique_ptr<UniformBuffer> obj = std::make_unique<UniformBuffer>();
            obj->init(size);
			return obj;
		}

		const SharedPtr<Buffer>& get_buffer() { return m_buffer; }

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
		SharedPtr<Buffer> m_buffer;
		void* m_mapped_data;
	};
}
