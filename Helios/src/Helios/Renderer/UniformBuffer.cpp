#include "UniformBuffer.h"

#include "Helios/Core/Application.h"

namespace Helios {
void UniformBuffer::init(size_t size) {
  const VulkanContext &context =
          Application::get().get_vulkan_manager()->get_context();

    m_buffer = Buffer::create(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  // Map the memory to the buffer, so we can write to it later
  vkMapMemory(context.device, m_buffer->get_vk_memory(), 0, size, 0,
              &m_mapped_data);
}
} // namespace Helios
