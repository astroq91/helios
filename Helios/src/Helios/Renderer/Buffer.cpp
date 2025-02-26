#include "Buffer.h"

#include "Helios/Core/Application.h"
#include "Helios/Vulkan/VulkanManager.h"

namespace Helios {
Buffer::~Buffer() {
    if (m_is_initialized) {
        auto buffer = m_buffer;
        auto buffer_memory = m_buffer_memory;
        auto useMappedMemory = m_use_mapped_memory;
        auto device =
            Application::get().get_vulkan_manager()->get_context().device;

        // Enqueue the destruction command
        Application::get().get_vulkan_manager()->enqueue_for_destruction([=]() {
            if (useMappedMemory) {
                vkUnmapMemory(device, buffer_memory);
            }
            vkDestroyBuffer(device, buffer, nullptr);
            vkFreeMemory(device, buffer_memory, nullptr);
        });
    }
}

void Buffer::init(VkDeviceSize size, VkBufferUsageFlags usage,
                  VkMemoryPropertyFlags properties, bool map_memory) {
    m_is_initialized = true;

    const VulkanContext& context =
        Application::get().get_vulkan_manager()->get_context();

    m_size = size;

    VkBufferCreateInfo buffer_info{};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size; // sizeof(Vertex) * vertices.size() ????
    buffer_info.usage = usage;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(context.device, &buffer_info, nullptr, &m_buffer) !=
        VK_SUCCESS) {
        HL_ERROR("Failed to create buffer!");
    }

    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(context.device, m_buffer, &mem_requirements);

    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_requirements.size;
    alloc_info.memoryTypeIndex = VulkanUtils::find_memory_type(
        mem_requirements.memoryTypeBits, properties, context.physical_device);

    if (vkAllocateMemory(context.device, &alloc_info, nullptr,
                         &m_buffer_memory) != VK_SUCCESS) {
        HL_ERROR("Failed to allocate buffer");
    }

    vkBindBufferMemory(context.device, m_buffer, m_buffer_memory, 0);

    m_use_mapped_memory = map_memory;
    if (m_use_mapped_memory) {
        vkMapMemory(context.device, m_buffer_memory, 0, size, 0,
                    &m_mapped_memory);
    }
}
} // namespace Helios
