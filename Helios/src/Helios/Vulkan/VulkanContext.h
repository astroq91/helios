#pragma once
#include <vector>
#include <volk/volk.h>

#include "Helios/Vulkan/VulkanUtils.h"

#ifdef _DEBUG
constexpr bool use_validation_layers = true;
#else
constexpr bool use_validation_layers = false;
#endif

namespace Helios {
struct VulkanContext {
    VkInstance instance;
    VkDevice device;
    VkSurfaceKHR surface;
    VkPhysicalDevice physical_device;

    VkDebugUtilsMessengerEXT debug_messenger;

    VkQueue graphics_queue;
    VkQueue present_queue;

    VkCommandPool command_pool;

    void Init() {
        // Initialize vulkan, and device related states
        VulkanUtils::create_instance(use_validation_layers,
                                     VulkanInstanceProps(), instance);
        VulkanUtils::setup_debug_messenger(instance, debug_messenger);
        VulkanUtils::create_surface(instance, surface);
        VulkanUtils::pick_physical_device(instance, surface, physical_device);
        VulkanUtils::create_logical_device(use_validation_layers, surface,
                                           physical_device, device,
                                           graphics_queue, present_queue);

        VulkanUtils::create_command_pool(device, physical_device, surface,
                                         command_pool);
    }

    ~VulkanContext() {
        vkDestroyCommandPool(device, command_pool, nullptr);

        vkDestroyDevice(device, nullptr);

        if (use_validation_layers) {
            VulkanUtils::destroy_debug_messenger(instance, debug_messenger,
                                                 nullptr);
        }

        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);
    }
};
} // namespace Helios
