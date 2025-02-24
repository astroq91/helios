#include "DescriptorSet.h"

#include "Helios/Core/Application.h"

namespace Helios {
void DescriptorSet::update_descriptor_set(
    const std::vector<DescriptorSpec> &descriptor_specs) {
  const VulkanContext &context =
          Application::get().get_vulkan_manager()->get_context();

  std::vector<VkDescriptorBufferInfo> buffer_infos(descriptor_specs.size());
  std::vector<VkDescriptorImageInfo> image_infos(descriptor_specs.size());
  std::vector<VkWriteDescriptorSet> descriptor_writes(descriptor_specs.size());

  for (size_t i = 0; i < descriptor_specs.size(); i++) {
      descriptor_writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptor_writes[i].dstSet = m_set;
      descriptor_writes[i].dstBinding = descriptor_specs[i].binding;
      descriptor_writes[i].dstArrayElement = descriptor_specs[i].dst_array_element;
      descriptor_writes[i].descriptorType = descriptor_specs[i].type;
      descriptor_writes[i].descriptorCount = descriptor_specs[i].descriptor_count;

    if (descriptor_specs[i].descriptor_class == DescriptorClass::Buffer) {
        buffer_infos[i].buffer = descriptor_specs[i].buffer->get_vk_buffer();
        buffer_infos[i].offset = 0;
        buffer_infos[i].range = descriptor_specs[i].buffer->get_vk_size();

        descriptor_writes[i].pBufferInfo = &buffer_infos[i];
    } else if (descriptor_specs[i].descriptor_class == DescriptorClass::Image) {
        image_infos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_infos[i].imageView = descriptor_specs[i].image_view;
        image_infos[i].sampler = descriptor_specs[i].sampler;

        descriptor_writes[i].pImageInfo = &image_infos[i];
    }

      descriptor_writes[i].pTexelBufferView =
        nullptr; // This can remain as it doesn't require external storage
  }

  vkUpdateDescriptorSets(context.device,
                         static_cast<uint32_t>(descriptor_writes.size()),
                         descriptor_writes.data(), 0, nullptr);
}

void DescriptorSet::init(const Ref<DescriptorPool> &pool,
                         const Ref<DescriptorSetLayout> &set_layout,
                         const std::vector<DescriptorSpec> &descriptor_specs) {
  const VulkanContext &context =
          Application::get().get_vulkan_manager()->get_context();

  VkDescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = pool->get_vk_pool();
    alloc_info.descriptorSetCount = 1;
    alloc_info.pSetLayouts = &set_layout->get_vk_layout();

  if (vkAllocateDescriptorSets(context.device, &alloc_info, &m_set) !=
      VK_SUCCESS) {
    HL_ERROR("Failed to allocate descriptor sets");
  }

    update_descriptor_set(descriptor_specs);
}
} // namespace Helios
