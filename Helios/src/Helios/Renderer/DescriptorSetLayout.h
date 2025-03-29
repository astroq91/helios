#pragma once

#include <volk/volk.h>

#include "Helios/Core/Core.h"

namespace Helios
{
	struct DescriptorSetLayoutBinding
	{
		uint32_t binding;
		VkDescriptorType type;
		VkShaderStageFlags stage;
		uint32_t descriptor_count;
	};

	class DescriptorSetLayout
	{
	public:
		static std::unique_ptr<DescriptorSetLayout> create_unique(const std::vector<DescriptorSetLayoutBinding>& layout_bindings)
		{
			std::unique_ptr<DescriptorSetLayout> obj = std::make_unique<DescriptorSetLayout>();
            obj->init(layout_bindings);
			return obj;
		}

		static SharedPtr<DescriptorSetLayout> create(const std::vector<DescriptorSetLayoutBinding>& layout_bindings)
		{
			SharedPtr<DescriptorSetLayout> obj = SharedPtr<DescriptorSetLayout>::create();
            obj->init(layout_bindings);
			return obj;
		}

		const VkDescriptorSetLayout& get_vk_layout() const { return m_layout; }

		DescriptorSetLayout() = default;
		~DescriptorSetLayout();

		DescriptorSetLayout(const DescriptorSetLayout&) = delete;
		DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;
		DescriptorSetLayout(DescriptorSetLayout&&) = delete;
		DescriptorSetLayout& operator=(DescriptorSetLayout&&) = delete;

	private:
		void init(const std::vector<DescriptorSetLayoutBinding>& layout_bindings);

	private:
		VkDescriptorSetLayout m_layout;

		bool m_is_initialized = false;
	};
}
