#include "DescriptorSetLayout.h"

#include "Helios/Core/Application.h"

namespace Helios {
DescriptorSetLayout::~DescriptorSetLayout() {
  if (m_is_initialized) {
    auto layout = m_layout;
    auto device = Application::get().get_vulkan_manager()->get_context().device;

    // Enqueue the destruction command
      Application::get().get_vulkan_manager()->enqueue_for_destruction(
              [=]() { vkDestroyDescriptorSetLayout(device, layout, nullptr); });
  }
}

void DescriptorSetLayout::init(
    const std::vector<DescriptorSetLayoutBinding> &layout_bindings) {
    m_is_initialized = true;

  const VulkanContext &context =
          Application::get().get_vulkan_manager()->get_context();

  // Creating the layout...
  std::vector<VkDescriptorSetLayoutBinding> bindings(layout_bindings.size());
  for (size_t i = 0; i < layout_bindings.size(); i++) {
    bindings[i].binding = layout_bindings[i].binding;
    bindings[i].descriptorCount = layout_bindings[i].descriptor_count;
    bindings[i].descriptorType =
        layout_bindings[i].type; // VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[i].stageFlags =
        layout_bindings[i].stage; // VK_SHADER_STAGE_VERTEX_BIT;
    bindings[i].pImmutableSamplers = nullptr;
  }

  VkDescriptorSetLayoutCreateInfo layout_info{};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
    layout_info.pBindings = bindings.data();

  if (vkCreateDescriptorSetLayout(context.device, &layout_info, nullptr,
                                  &m_layout) != VK_SUCCESS) {
    HL_ERROR("Failed to create descriptor set layout");
  }
}
} // namespace Helios
