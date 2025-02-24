#include "CommandBuffer.h"

#include "Helios/Core/Application.h"

namespace Helios {
void CommandBuffer::init() {
  const VulkanContext &context =
          Application::get().get_vulkan_manager()->get_context();

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = context.command_pool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = 1;

  if (vkAllocateCommandBuffers(context.device, &allocInfo, &m_buffer) !=
      VK_SUCCESS) {
    HL_ERROR("Failed to allocate command buffer!");
  }
}
} // namespace Helios
