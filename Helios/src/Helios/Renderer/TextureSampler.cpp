#include "TextureSampler.h"

#include "Helios/Core/Application.h"

namespace Helios {
TextureSampler::~TextureSampler() {
  if (m_is_initialized) {
    auto sampler = m_sampler;
    auto device = Application::get().get_vulkan_manager()->get_context().device;

    // Enqueue the destruction command
      Application::get().get_vulkan_manager()->enqueue_for_destruction(
              [=]() { vkDestroySampler(device, sampler, nullptr); });
  }
}

void TextureSampler::init() {
    m_is_initialized = true;

  const VulkanContext &context =
          Application::get().get_vulkan_manager()->get_context();

  VkSamplerCreateInfo sampler_info{};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;

    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

  VkPhysicalDeviceProperties properties{};
  vkGetPhysicalDeviceProperties(context.physical_device, &properties);

    sampler_info.anisotropyEnable = VK_TRUE;
    sampler_info.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

    sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
    sampler_info.unnormalizedCoordinates = VK_FALSE;

    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;

    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.mipLodBias = 0.0f;
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = 0.0f;

  if (vkCreateSampler(context.device, &sampler_info, nullptr, &m_sampler) !=
      VK_SUCCESS) {
    HL_ERROR("Failed to create texture sampler");
  }
}
} // namespace Helios
