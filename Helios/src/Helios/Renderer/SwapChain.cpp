#include "SwapChain.h"

#include "Helios/Core/Application.h"

namespace Helios {
SwapChain::~SwapChain() {
  if (m_is_initialized) {
    // Don't defer destruction for the swap chain
    auto device = Application::get().get_vulkan_manager()->get_context().device;

    for (auto imageView : m_image_views) {
      vkDestroyImageView(device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(device, m_swapchain, nullptr);
  }
}

void SwapChain::init() {
    m_is_initialized = true;

  const VulkanContext &context =
          Application::get().get_vulkan_manager()->get_context();

  SwapchainSupportDetails swapchain_support = VulkanUtils::query_swapchain_support(
          context.physical_device, context.surface);

    m_surface_format =
            VulkanUtils::choose_swap_surface_format(swapchain_support.formats);
    m_present_mode =
            VulkanUtils::choose_swap_present_mode(swapchain_support.present_modes);
  VkExtent2D extent =
          VulkanUtils::choose_swap_extent(swapchain_support.capabilities);

  uint32_t image_count = swapchain_support.capabilities.minImageCount + 1;
  if (swapchain_support.capabilities.maxImageCount > 0 &&
      image_count > swapchain_support.capabilities.maxImageCount) {
      image_count = swapchain_support.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = context.surface;

    create_info.minImageCount = image_count;
    create_info.imageFormat = m_surface_format.format;
    create_info.imageColorSpace = m_surface_format.colorSpace;
    create_info.imageExtent = extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  QueueFamilyIndices indices =
          VulkanUtils::find_queue_families(context.physical_device, context.surface);
  uint32_t queueFamilyIndices[] = {indices.graphics_family.value(),
                                   indices.present_family.value()};

  if (indices.graphics_family != indices.present_family) {
      create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
      create_info.queueFamilyIndexCount = 2;
      create_info.pQueueFamilyIndices = queueFamilyIndices;
  } else {
      create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

    create_info.preTransform = swapchain_support.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = m_present_mode;
    create_info.clipped = VK_TRUE;

    create_info.oldSwapchain = VK_NULL_HANDLE;

  if (vkCreateSwapchainKHR(context.device, &create_info, nullptr,
                           &m_swapchain) != VK_SUCCESS) {
    HL_ERROR("Failed to create swap chain!");
  }

  vkGetSwapchainImagesKHR(context.device, m_swapchain, &image_count, nullptr);
  m_images.resize(image_count);
  vkGetSwapchainImagesKHR(context.device, m_swapchain, &image_count,
                          m_images.data());

    m_image_format = m_surface_format.format;
    m_extent = extent;

  // create the image views
  m_image_views.resize(m_images.size());
  for (size_t i = 0; i < m_images.size(); i++) {
    // TODO: Maybe shift to using image class?
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = m_images[i];
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = m_image_format;
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(context.device, &createInfo, nullptr,
                          &m_image_views[i]) != VK_SUCCESS) {
      HL_ERROR("Failed to create image views!");
    }
  }
}
} // namespace Helios
