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
		static Unique<DescriptorSetLayout> create_unique(const std::vector<DescriptorSetLayoutBinding>& layout_bindings)
		{
			Unique<DescriptorSetLayout> obj = make_unique<DescriptorSetLayout>();
            obj->init(layout_bindings);
			return obj;
		}

		static Ref<DescriptorSetLayout> create(const std::vector<DescriptorSetLayoutBinding>& layout_bindings)
		{
			Ref<DescriptorSetLayout> obj = make_ref<DescriptorSetLayout>();
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
