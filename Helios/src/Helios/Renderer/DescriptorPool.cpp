#include "DescriptorPool.h"

#include "Helios/Core/Application.h"

namespace Helios {
DescriptorPool::~DescriptorPool() {
  if (m_is_initialized) {
    auto pool = m_pool;
    auto device = Application::get().get_vulkan_manager()->get_context().device;

    // Enqueue the destruction command
      Application::get().get_vulkan_manager()->enqueue_for_destruction(
              [=]() { vkDestroyDescriptorPool(device, pool, nullptr); });
  }
}

void DescriptorPool::init(uint32_t max_sets,
                          const std::vector<VkDescriptorPoolSize> &pool_sizes,
                          VkDescriptorPoolCreateFlags flags) {
    m_is_initialized = true;

  const VulkanContext &context =
          Application::get().get_vulkan_manager()->get_context();

  VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
    pool_info.pPoolSizes = pool_sizes.data();
    pool_info.maxSets = max_sets;
    pool_info.flags = flags;

  if (vkCreateDescriptorPool(context.device, &pool_info, nullptr, &m_pool) !=
      VK_SUCCESS) {
    HL_ERROR("Failed to create descriptor pool");
  }
}
} // namespace Helios
