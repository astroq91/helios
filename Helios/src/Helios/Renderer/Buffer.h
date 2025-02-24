#pragma once
#include <vulkan/vulkan.h>

#include "Resource.h"
#include "Helios/Core/Core.h"

namespace Helios
{
	class Buffer : public Resource
	{
	public:
		static Ref<Buffer> create(VkDeviceSize size, VkBufferUsageFlags usage,
                                  VkMemoryPropertyFlags properties,
                                  bool map_memory = false)
		{
			Ref<Buffer> b = make_ref<Buffer>();
            b->init_uuid();
            b->init(size, usage, properties, map_memory);
			return b;
		}

		static Unique<Buffer> create_unique(VkDeviceSize size, VkBufferUsageFlags usage,
                                            VkMemoryPropertyFlags properties,
                                            bool map_memory = false)
		{
			Unique<Buffer> b = make_unique<Buffer>();
            b->init_uuid();
            b->init(size, usage, properties, map_memory);
			return b;
		}

		const VkBuffer& get_vk_buffer() const { return m_buffer; }
		const VkDeviceMemory& get_vk_memory() const { return m_buffer_memory; }
		const VkDeviceSize& get_vk_size() const { return m_size; }

		/**
		 * \brief get the mapped memory. Only valid if mapMemory=true during buffer creation.
		 * \return The mapped memory.
		 */
		void* get_mapped_memory() const { return m_mapped_memory; }

		~Buffer();

		Buffer() = default;

		Buffer(const Buffer&) = delete;
		Buffer& operator=(const Buffer&) = delete;
		Buffer(Buffer&&) = delete;
		Buffer& operator=(Buffer&&) = delete;

	private:
		void init(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, bool map_memory);

	private:
		VkBuffer m_buffer;
		VkDeviceMemory m_buffer_memory;
		VkDeviceSize m_size;

		// Optional
		void* m_mapped_memory;
		bool m_use_mapped_memory;

		bool m_is_initialized = false;
	};
}
