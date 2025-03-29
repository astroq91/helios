#pragma once

#include <volk/volk.h>

#include "Helios/Core/Core.h"

namespace Helios
{
	class DescriptorPool
	{
	public:
		static SharedPtr<DescriptorPool> create(uint32_t max_sets, const std::vector<VkDescriptorPoolSize>& pool_sizes,
                                          VkDescriptorPoolCreateFlags flags = 0)
		{
			SharedPtr<DescriptorPool> obj = SharedPtr<DescriptorPool>::create();
            obj->init(max_sets, pool_sizes, flags);
			return obj;
		}

		static std::unique_ptr<DescriptorPool> CreateUnique(uint32_t max_sets, const std::vector<VkDescriptorPoolSize>& pool_sizes,
                                                   VkDescriptorPoolCreateFlags flags = 0)
		{
			std::unique_ptr<DescriptorPool> obj = std::make_unique<DescriptorPool>();
            obj->init(max_sets, pool_sizes, flags);
			return obj;
		}

		VkDescriptorPool get_vk_pool() const { return m_pool; }

		DescriptorPool() = default;
		~DescriptorPool();

		DescriptorPool(const DescriptorPool&) = delete;
		DescriptorPool& operator=(const DescriptorPool&) = delete;
		DescriptorPool(DescriptorPool&&) = delete;
		DescriptorPool& operator=(DescriptorPool&&) = delete;

	private:
		void init(uint32_t max_sets, const std::vector<VkDescriptorPoolSize>& pool_sizes,
                  VkDescriptorPoolCreateFlags flags);

	private:
		VkDescriptorPool m_pool;

		bool m_is_initialized = false;
	};
}
