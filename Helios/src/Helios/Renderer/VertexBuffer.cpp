#include "VertexBuffer.h"

#include "Buffer.h"
#include "Helios/Core/Application.h"

namespace Helios {
void VertexBuffer::insert_memory(void *data, size_t size, uint32_t offset) {
  const VulkanContext &context =
          Application::get().get_vulkan_manager()->get_context();
  const Renderer &renderer = Application::get().get_renderer();

  std::unique_ptr<Buffer> staging_buffer =
          Buffer::create_unique(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  void *data_map;
  vkMapMemory(context.device, staging_buffer->get_vk_memory(), 0, size, 0,
              &data_map);
  memcpy(data_map, data, size);
  vkUnmapMemory(context.device, staging_buffer->get_vk_memory());

  // Only give a command buffer if the global one is currently recording
    VulkanUtils::copy_buffer(
            staging_buffer->get_vk_buffer(), m_buffer->get_vk_buffer(), size, context.device,
            context.command_pool, context.graphics_queue, VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
            renderer.is_recording()
            ? renderer.get_current_command_buffer()->get_command_buffer()
            : VK_NULL_HANDLE);
}

void VertexBuffer::init(void *data, size_t size) {
    m_buffer = Buffer::create_unique(size,
                                   VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  if (data != nullptr) {
      insert_memory(data, size);
  }
}
} // namespace Helios
