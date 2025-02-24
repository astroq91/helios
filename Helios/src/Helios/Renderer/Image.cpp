#include "Image.h"

#include "Helios/Core/Application.h"

namespace Helios {
Image::~Image() {
  if (m_is_initialized) {
    auto image_view = m_image_view;
    auto image = m_image;
    auto memory = m_image_memory;
    auto device = Application::get().get_vulkan_manager()->get_context().device;

    // Enqueue the destruction command
      Application::get().get_vulkan_manager()->enqueue_for_destruction([=]() {
          vkDestroyImageView(device, image_view, nullptr);
          vkDestroyImage(device, image, nullptr);
          vkFreeMemory(device, memory, nullptr);
      });
  }
}

void Image::init(const ImageSpec &spec) {
    m_is_initialized = true;
  const VulkanContext &context =
          Application::get().get_vulkan_manager()->get_context();

    m_width = spec.width;
    m_height = spec.height;

  VkImageCreateInfo image_info{};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = m_width;
    image_info.extent.height = m_height;
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;

    image_info.format = spec.format;
    image_info.tiling = spec.tiling;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = spec.usage;
    image_info.sharingMode =
      VK_SHARING_MODE_EXCLUSIVE; // Only used by one queue family
  image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.flags = 0;

  if (vkCreateImage(context.device, &image_info, nullptr, &m_image) !=
      VK_SUCCESS) {
    HL_ERROR("Failed to create image");
  }

  VkMemoryRequirements mem_requirements;
  vkGetImageMemoryRequirements(context.device, m_image, &mem_requirements);

    m_image_size = mem_requirements.size;

  VkMemoryAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.allocationSize = mem_requirements.size;
    allocate_info.memoryTypeIndex =
            VulkanUtils::find_memory_type(mem_requirements.memoryTypeBits,
                                          spec.memory_property, context.physical_device);

  if (vkAllocateMemory(context.device, &allocate_info, nullptr,
                       &m_image_memory) != VK_SUCCESS) {
    HL_ERROR("Failed to allocate image memory");
  }

  vkBindImageMemory(context.device, m_image, m_image_memory, 0);

  VkImageViewCreateInfo view_info{};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = m_image;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = spec.format;
    view_info.subresourceRange.aspectMask = spec.aspect_flags;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

  if (vkCreateImageView(context.device, &view_info, nullptr, &m_image_view) !=
      VK_SUCCESS) {
    HL_ERROR("Failed to create image view");
  }
}
} // namespace Helios
