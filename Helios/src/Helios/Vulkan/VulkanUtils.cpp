#include "VulkanUtils.h"

#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>
#include <vector>
#include <volk/volk.h>
#include <vulkan/vulkan_core.h>

#include "Helios/Core/Application.h"

const std::vector<const char*> g_validation_layers = {
    "VK_LAYER_KHRONOS_validation"};

const std::vector<const char*> g_device_extensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
    VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME,
    VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
    VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME};

VkResult create_debug_utils_messenger_ext(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* p_create_info,
    const VkAllocationCallbacks* p_allocator,
    VkDebugUtilsMessengerEXT* p_debug_messenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, p_create_info, p_allocator, p_debug_messenger);
    } else {
        HL_ERROR("Not present");
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL
debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
               VkDebugUtilsMessageTypeFlagsEXT message_type,
               const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
               void* p_user_data) {
    switch (message_severity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        HL_INFO("[Vulkan] {0}", p_callback_data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        HL_WARN("[Vulkan] {0}", p_callback_data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        HL_ERROR("[Vulkan] {0}", p_callback_data->pMessage);
        break;
    default:
        HL_INFO("[Vulkan] {0}", p_callback_data->pMessage);
    }

    return VK_FALSE;
}

namespace Helios {
void VulkanUtils::create_instance(bool use_validation_layers,
                                  const VulkanInstanceProps& props,
                                  VkInstance& instance) {

    if (volkInitialize() != VK_SUCCESS) {
        HL_ERROR("Could not find the loader for Vulkan!");
        return;
    }

    if (use_validation_layers && !check_validation_layer_support()) {
        HL_WARN("Validation layers are not available.");
        use_validation_layers = false;
    }

    // TODO: Check if Vulkan 1.2 is supported

    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = props.app_name.c_str();
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = props.engine_name.c_str();
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;

    auto extensions = get_required_extensions(use_validation_layers);
    create_info.enabledExtensionCount =
        static_cast<uint32_t>(extensions.size());
    create_info.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
    if (use_validation_layers) {
        create_info.enabledLayerCount =
            static_cast<uint32_t>(g_validation_layers.size());
        create_info.ppEnabledLayerNames = g_validation_layers.data();

        populate_debug_messenger_create_info(debug_create_info);
        create_info.pNext =
            (VkDebugUtilsMessengerCreateInfoEXT*)&debug_create_info;
    } else {
        create_info.enabledLayerCount = 0;

        create_info.pNext = nullptr;
    }

    if (vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS) {
        HL_ERROR("Failed to create instance!");
    }

    volkLoadInstance(instance);
}

void VulkanUtils::populate_debug_messenger_create_info(
    VkDebugUtilsMessengerCreateInfoEXT& create_info) {
    create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info.pfnUserCallback = debug_callback;
}

void VulkanUtils::destroy_debug_messenger(
    VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger,
    const VkAllocationCallbacks* p_allocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debug_messenger, p_allocator);
    }
}

void VulkanUtils::setup_debug_messenger(
    VkInstance instance, VkDebugUtilsMessengerEXT& debug_messenger) {
    VkDebugUtilsMessengerCreateInfoEXT create_info;
    populate_debug_messenger_create_info(create_info);

    if (create_debug_utils_messenger_ext(instance, &create_info, nullptr,
                                         &debug_messenger) != VK_SUCCESS) {
        HL_ERROR("Failed to set up debug messenger!");
    }
}

void VulkanUtils::create_surface(VkInstance instance, VkSurfaceKHR& surface) {
    if (glfwCreateWindowSurface(instance,
                                Application::get().get_native_window(), nullptr,
                                &surface) != VK_SUCCESS) {
        HL_ERROR("Failed to create window surface!");
    }
}
void VulkanUtils::pick_physical_device(VkInstance instance,
                                       VkSurfaceKHR surface,
                                       VkPhysicalDevice& physical_device) {
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, nullptr);

    if (device_count == 0) {
        HL_ERROR("Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(instance, &device_count, devices.data());

    for (const auto& device : devices) {
        if (is_device_suitable(device, surface)) {
            physical_device = device;
            break;
        }
    }

    if (physical_device == VK_NULL_HANDLE) {
        HL_ERROR("Failed to find a suitable GPU!");
    }
}

void VulkanUtils::create_logical_device(bool use_validation_layers,
                                        VkSurfaceKHR surface,
                                        VkPhysicalDevice physical_device,
                                        VkDevice& device,
                                        VkQueue& graphics_queue,
                                        VkQueue& present_queue) {
    QueueFamilyIndices indices = find_queue_families(physical_device, surface);

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    std::set<uint32_t> unique_queue_families = {indices.graphics_family.value(),
                                                indices.present_family.value()};

    float queue_priority = 1.0f;
    for (uint32_t queue_family : unique_queue_families) {
        VkDeviceQueueCreateInfo queue_create_info{};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = queue_family;
        queue_create_info.queueCount = 1;
        queue_create_info.pQueuePriorities = &queue_priority;
        queue_create_infos.push_back(queue_create_info);
    }

    // Vulkan 1.2 features

    VkPhysicalDeviceVulkan12Features vulkan_12_features{};
    vulkan_12_features.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    vulkan_12_features.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
    vulkan_12_features.descriptorIndexing = VK_TRUE;

    // Dynamic rendering
    VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering{};
    dynamic_rendering.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
    dynamic_rendering.pNext = &vulkan_12_features;
    dynamic_rendering.dynamicRendering = VK_TRUE;

    // Features
    VkPhysicalDeviceFeatures2 device_features_2{};
    device_features_2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    device_features_2.pNext = &dynamic_rendering;
    device_features_2.features.samplerAnisotropy = VK_TRUE;
    device_features_2.features.shaderSampledImageArrayDynamicIndexing = VK_TRUE;

    VkDeviceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.pNext = &device_features_2;

    create_info.queueCreateInfoCount =
        static_cast<uint32_t>(queue_create_infos.size());
    create_info.pQueueCreateInfos = queue_create_infos.data();

    create_info.pEnabledFeatures = nullptr;

    create_info.enabledExtensionCount =
        static_cast<uint32_t>(g_device_extensions.size());
    create_info.ppEnabledExtensionNames = g_device_extensions.data();

    if (use_validation_layers) {
        create_info.enabledLayerCount =
            static_cast<uint32_t>(g_validation_layers.size());
        create_info.ppEnabledLayerNames = g_validation_layers.data();
    } else {
        create_info.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physical_device, &create_info, nullptr, &device) !=
        VK_SUCCESS) {
        HL_ERROR("Failed to create logical device!");
    }

    volkLoadDevice(device);

    vkGetDeviceQueue(device, indices.graphics_family.value(), 0,
                     &graphics_queue);
    vkGetDeviceQueue(device, indices.present_family.value(), 0, &present_queue);
}

VkSurfaceFormatKHR VulkanUtils::choose_swap_surface_format(
    const std::vector<VkSurfaceFormatKHR>& available_formats) {
    for (const auto& available_format : available_formats) {
        if (available_format.format == VK_FORMAT_R8G8B8A8_SRGB &&
            available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return available_format;
        }
    }

    return available_formats[0];
}

VkPresentModeKHR VulkanUtils::choose_swap_present_mode(
    const std::vector<VkPresentModeKHR>& available_present_modes,
    VkPresentModeKHR preferred_mode) {
    for (const auto& available_present_mode : available_present_modes) {
        if (available_present_mode == preferred_mode) {
            return available_present_mode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D
VulkanUtils::choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width !=
        std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(Application::get().get_native_window(), &width,
                               &height);

        VkExtent2D actual_extent = {static_cast<uint32_t>(width),
                                    static_cast<uint32_t>(height)};

        actual_extent.width =
            std::clamp(actual_extent.width, capabilities.minImageExtent.width,
                       capabilities.maxImageExtent.width);
        actual_extent.height =
            std::clamp(actual_extent.height, capabilities.minImageExtent.height,
                       capabilities.maxImageExtent.height);

        return actual_extent;
    }
}

SwapchainSupportDetails
VulkanUtils::query_swapchain_support(VkPhysicalDevice device,
                                     VkSurfaceKHR surface) {
    SwapchainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
                                              &details.capabilities);

    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count,
                                         nullptr);

    if (format_count != 0) {
        details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count,
                                             details.formats.data());
    }

    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
                                              &present_mode_count, nullptr);

    if (present_mode_count != 0) {
        details.present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            device, surface, &present_mode_count, details.present_modes.data());
    }

    return details;
}

VkFormat VulkanUtils::find_supported_format(
    VkPhysicalDevice device, const std::vector<VkFormat>& candidates,
    VkImageTiling tiling, VkFormatFeatureFlags features) {
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(device, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR &&
            (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
                   (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    HL_ERROR("Failed to find supported format!");
}

VkFormat VulkanUtils::find_depth_format(VkPhysicalDevice device) {
    return find_supported_format(
        device,
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
         VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

bool VulkanUtils::has_stencil_component(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
           format == VK_FORMAT_D24_UNORM_S8_UINT;
}

bool VulkanUtils::is_device_suitable(VkPhysicalDevice device,
                                     VkSurfaceKHR surface) {
    QueueFamilyIndices indices = find_queue_families(device, surface);

    bool extensions_supported = check_device_extension_support(device);

    bool swap_chain_adequate = false;
    if (extensions_supported) {
        SwapchainSupportDetails swap_chain_support =
            query_swapchain_support(device, surface);
        swap_chain_adequate = !swap_chain_support.formats.empty() &&
                              !swap_chain_support.present_modes.empty();
    }

    VkPhysicalDeviceFeatures supported_features;
    vkGetPhysicalDeviceFeatures(device, &supported_features);

    VkPhysicalDeviceFeatures2 supported_features_2{};
    supported_features_2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    vkGetPhysicalDeviceFeatures2(device, &supported_features_2);

    return indices.is_complete() && extensions_supported &&
           swap_chain_adequate && supported_features.samplerAnisotropy &&
           supported_features_2.features.shaderSampledImageArrayDynamicIndexing;
}

bool VulkanUtils::check_device_extension_support(VkPhysicalDevice device) {
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count,
                                         nullptr);

    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count,
                                         available_extensions.data());

    std::set<std::string> required_extensions(g_device_extensions.begin(),
                                              g_device_extensions.end());

    for (const auto& extension : available_extensions) {
        required_extensions.erase(extension.extensionName);
    }

    return required_extensions.empty();
}

QueueFamilyIndices VulkanUtils::find_queue_families(VkPhysicalDevice device,
                                                    VkSurfaceKHR surface) {
    QueueFamilyIndices indices;

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count,
                                             nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count,
                                             queue_families.data());

    int i = 0;
    for (const auto& queue_family : queue_families) {
        if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphics_family = i;
        }

        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface,
                                             &present_support);

        if (present_support) {
            indices.present_family = i;
        }

        if (indices.is_complete()) {
            break;
        }

        i++;
    }

    return indices;
}

std::vector<const char*>
VulkanUtils::get_required_extensions(bool use_validation_layers) {
    uint32_t glfw_extension_count = 0;
    const char** glfw_extensions;
    glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    std::vector<const char*> extensions(glfw_extensions,
                                        glfw_extensions + glfw_extension_count);

    if (use_validation_layers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

bool VulkanUtils::check_validation_layer_support() {
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

    std::vector<VkLayerProperties> available_layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

    for (const char* layer_name : g_validation_layers) {
        bool layer_found = false;

        for (const auto& layer_properties : available_layers) {
            if (strcmp(layer_name, layer_properties.layerName) == 0) {
                layer_found = true;
                break;
            }
        }

        if (!layer_found) {
            return false;
        }
    }

    return true;
}

void VulkanUtils::create_command_pool(VkDevice device,
                                      VkPhysicalDevice physical_device,
                                      VkSurfaceKHR surface,
                                      VkCommandPool& command_pool) {
    QueueFamilyIndices queue_family_indices =
        find_queue_families(physical_device, surface);

    VkCommandPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_info.queueFamilyIndex = queue_family_indices.graphics_family.value();

    if (vkCreateCommandPool(device, &pool_info, nullptr, &command_pool) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics command pool!");
    }
}

void VulkanUtils::create_semaphores(VkDevice device, size_t count,
                                    std::vector<VkSemaphore>& semaphores) {
    semaphores.resize(count);

    VkSemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for (size_t i = 0; i < count; i++) {
        if (vkCreateSemaphore(device, &semaphore_info, nullptr,
                              &semaphores[i]) != VK_SUCCESS) {
            HL_ERROR("Failed to create semaphore!");
        }
    }
}

void VulkanUtils::create_fences(VkDevice device, size_t count,
                                std::vector<VkFence>& fences) {
    fences.resize(count);

    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < count; i++) {
        if (vkCreateFence(device, &fence_info, nullptr, &fences[i]) !=
            VK_SUCCESS) {
            HL_ERROR("Failed to create fence!");
        }
    }
}

void VulkanUtils::copy_buffer_to_image(VkCommandBuffer command_buffer,
                                       VkBuffer buffer, VkImage image,
                                       uint32_t width, uint32_t height,
                                       uint32_t index) {
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .mipLevel = 0,
        .baseArrayLayer = index,
        .layerCount = 1,
    };

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        width,
        height,
        1,
    };

    vkCmdCopyBufferToImage(command_buffer, buffer, image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void VulkanUtils::copy_image_to_buffer(VkCommandBuffer command_buffer,
                                       VkImage image, uint32_t width,
                                       uint32_t height,
                                       VkImageLayout image_layout,
                                       VkBuffer buffer) {
    VkBufferImageCopy region{};

    region.imageSubresource = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .mipLevel = 0,
        .baseArrayLayer = 0,
        .layerCount = 1,
    };

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {.width = width, .height = height, .depth = 1};

    vkCmdCopyImageToBuffer(command_buffer, image, image_layout, buffer, 1,
                           &region);
}

void VulkanUtils::transition_image_layout(
    const TransitionImageLayoutSpec& spec) {
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = spec.old_layout;
    barrier.newLayout = spec.new_layout;

    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.image = spec.image;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = spec.index;
    barrier.subresourceRange.layerCount = 1;

    if (spec.new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (VulkanUtils::has_stencil_component(spec.format)) {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    } else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    barrier.srcAccessMask = spec.src_access_mask;
    barrier.dstAccessMask = spec.dst_access_mask;

    vkCmdPipelineBarrier(spec.command_buffer, spec.src_stage_mask,
                         spec.dst_stage_mask, spec.dependency_flags, 0, nullptr,
                         0, nullptr, 1, &barrier);
}

VkCommandBuffer
VulkanUtils::begin_single_time_commands(VkDevice device,
                                        VkCommandPool command_pool) {
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = command_pool;
    alloc_info.commandBufferCount = 1;

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(device, &alloc_info,
                             &command_buffer); // Handle bad result?

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(command_buffer, &begin_info);

    return command_buffer;
}

void VulkanUtils::end_single_time_commands(VkCommandBuffer command_buffer,
                                           VkDevice device,
                                           VkCommandPool command_pool,
                                           VkQueue queue) // Use graphics queue
{
    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;

    vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);

    vkFreeCommandBuffers(device, command_pool, 1, &command_buffer);
}

void VulkanUtils::copy_buffer(VkBuffer src_buffer, VkBuffer dst_buffer,
                              VkDeviceSize size, VkDevice device,
                              VkCommandPool command_pool, VkQueue queue,
                              VkAccessFlags src_access_mask,
                              VkAccessFlags dst_access_mask,
                              VkPipelineStageFlags src_stage_mask,
                              VkPipelineStageFlags dst_stage_mask,
                              VkCommandBuffer command_buffer) {
    VkCommandBuffer command_buffer_local;
    if (command_buffer == VK_NULL_HANDLE) {
        command_buffer_local = begin_single_time_commands(device, command_pool);
    } else {
        command_buffer_local = command_buffer;
    }

    VkBufferCopy copy_region{};
    copy_region.size = size;
    vkCmdCopyBuffer(command_buffer_local, src_buffer, dst_buffer, 1,
                    &copy_region);

    VkBufferMemoryBarrier buffer_memory_barrier = {};
    buffer_memory_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    buffer_memory_barrier.srcAccessMask = src_access_mask;
    buffer_memory_barrier.dstAccessMask = dst_access_mask;
    buffer_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    buffer_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    buffer_memory_barrier.buffer = dst_buffer;
    // The buffer being accessed
    buffer_memory_barrier.offset = 0; // Offset within the buffer
    buffer_memory_barrier.size = copy_region.size;
    // Size of the memory range within the buffer, VK_WHOLE_SIZE means the
    // entire buffer

    // Insert the barrier into the command buffer
    vkCmdPipelineBarrier(
        command_buffer_local,
        src_stage_mask, // Pipeline stages the operations occur in before the
        // barrier
        dst_stage_mask, // Pipeline stages the operations will wait on this
                        // barrier
        0,              // Dependency flags
        0, nullptr,     // No memory barriers
        1, &buffer_memory_barrier, // Buffer memory barrier count and pointer
        0, nullptr                 // No image memory barriers
    );

    if (command_buffer == VK_NULL_HANDLE) {
        end_single_time_commands(command_buffer_local, device, command_pool,
                                 queue);
    }
}

uint32_t VulkanUtils::find_memory_type(uint32_t type_filter,
                                       VkMemoryPropertyFlags properties,
                                       VkPhysicalDevice physical_device) {
    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_properties);

    for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
        if (type_filter & (1 << i) &&
            (mem_properties.memoryTypes[i].propertyFlags & properties) ==
                properties) {
            return i;
        }
    }

    HL_ERROR("Failed to find suitable memory type");

    return 0;
}

void VulkanUtils::cmd_begin_rendering_khr(VkInstance instance,
                                          VkCommandBuffer command_buffer,
                                          VkRenderingInfoKHR* render_info) {
    auto func = (PFN_vkCmdBeginRenderingKHR)vkGetInstanceProcAddr(
        instance, "vkCmdBeginRenderingKHR");
    if (func != nullptr) {
        func(command_buffer, render_info);
    } else {
        HL_ERROR("Failed to load vkCmdBeginRenderingKHR");
    }
}

void VulkanUtils::cmd_end_rendering_khr(VkInstance instance,
                                        VkCommandBuffer command_buffer) {
    auto func = (PFN_vkCmdEndRenderingKHR)vkGetInstanceProcAddr(
        instance, "vkCmdEndRenderingKHR");
    if (func != nullptr) {
        func(command_buffer);
    } else {
        HL_ERROR("Failed to load vkCmdEndRenderingKHR");
    }
}

std::vector<char> ReadFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        HL_ERROR("Failed to open file!");
    }

    size_t file_size = (size_t)file.tellg();
    std::vector<char> buffer(file_size);

    file.seekg(0);
    file.read(buffer.data(), file_size);

    file.close();

    return buffer;
}
} // namespace Helios
