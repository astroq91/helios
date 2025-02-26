#pragma once

#include <volk/volk.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <optional>
#include <string>
#include <vector>

struct QueueFamilyIndices;
struct SwapChainSupportDetails;

namespace Helios {
struct VulkanInstanceProps {
    VulkanInstanceProps() = default;

    std::string app_name = "Helios";
    std::string engine_name = "Helios";
};

struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
};

struct QueueFamilyIndices {
    std::optional<uint32_t> graphics_family;
    std::optional<uint32_t> present_family;

    bool is_complete() {
        return graphics_family.has_value() && present_family.has_value();
    }
};

struct TransitionImageLayoutSpec {
    VkImage image;
    VkFormat format;
    VkImageLayout old_layout;
    VkImageLayout new_layout;

    VkAccessFlags src_access_mask;
    VkAccessFlags dst_access_mask;

    VkPipelineStageFlags src_stage_mask;
    VkPipelineStageFlags dst_stage_mask;

    VkDependencyFlags dependency_flags;

    VkCommandBuffer command_buffer;
};

class VulkanUtils {
  public:
    static void create_instance(bool use_validation_layers,
                                const VulkanInstanceProps& props,
                                VkInstance& instance);
    static void create_surface(VkInstance instance, VkSurfaceKHR& surface);
    static void create_logical_device(bool use_validation_layers,
                                      VkSurfaceKHR surface,
                                      VkPhysicalDevice physical_device,
                                      VkDevice& device, VkQueue& graphics_queue,
                                      VkQueue& present_queue);
    static void create_command_pool(VkDevice device,
                                    VkPhysicalDevice physical_device,
                                    VkSurfaceKHR surface,
                                    VkCommandPool& command_pool);

    static void create_semaphores(VkDevice device, size_t count,
                                  std::vector<VkSemaphore>& semaphores);
    static void create_fences(VkDevice device, size_t count,
                              std::vector<VkFence>& fences);

    /**
     * \brief Copy a src buffer to a dst buffer.
     * \param src_buffer
     * \param dst_buffer
     * \param size
     * \param device
     * \param command_pool
     * \param queue
     * \param src_access_mask
     * \param dst_access_mask
     * \param src_stage_mask
     * \param dst_stage_mask
     * \param command_buffer Optional command buffer. Use this if one is already
     * recording.
     */
    static void copy_buffer(VkBuffer src_buffer, VkBuffer dst_buffer,
                            VkDeviceSize size, VkDevice device,
                            VkCommandPool command_pool, VkQueue queue,
                            VkAccessFlags src_access_mask,
                            VkAccessFlags dst_access_mask,
                            VkPipelineStageFlags src_stage_mask,
                            VkPipelineStageFlags dst_stage_mask,
                            VkCommandBuffer command_buffer = VK_NULL_HANDLE);

    static VkCommandBuffer
    begin_single_time_commands(VkDevice device, VkCommandPool command_pool);
    static void end_single_time_commands(VkCommandBuffer command_buffer,
                                         VkDevice device,
                                         VkCommandPool command_pool,
                                         VkQueue queue);

    static uint32_t find_memory_type(uint32_t type_filter,
                                     VkMemoryPropertyFlags properties,
                                     VkPhysicalDevice physical_device);

    static std::vector<const char*>
    get_required_extensions(bool use_validation_layers);
    static bool check_validation_layer_support();
    static QueueFamilyIndices find_queue_families(VkPhysicalDevice device,
                                                  VkSurfaceKHR surface);

    static void pick_physical_device(VkInstance instance, VkSurfaceKHR surface,
                                     VkPhysicalDevice& physical_device);
    static bool is_device_suitable(VkPhysicalDevice device,
                                   VkSurfaceKHR surface);
    static bool check_device_extension_support(VkPhysicalDevice device);

    static void
    setup_debug_messenger(VkInstance instance,
                          VkDebugUtilsMessengerEXT& debug_messenger);
    static void populate_debug_messenger_create_info(
        VkDebugUtilsMessengerCreateInfoEXT& create_info);
    static void
    destroy_debug_messenger(VkInstance instance,
                            VkDebugUtilsMessengerEXT debug_messenger,
                            const VkAllocationCallbacks* p_allocator);

    static VkSurfaceFormatKHR choose_swap_surface_format(
        const std::vector<VkSurfaceFormatKHR>& available_formats);
    static VkPresentModeKHR choose_swap_present_mode(
        const std::vector<VkPresentModeKHR>& available_present_modes);
    static VkExtent2D
    choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities);
    static SwapchainSupportDetails
    query_swapchain_support(VkPhysicalDevice device, VkSurfaceKHR surface);

    static VkFormat
    find_supported_format(VkPhysicalDevice device,
                          const std::vector<VkFormat>& candidates,
                          VkImageTiling tiling, VkFormatFeatureFlags features);

    static VkFormat find_depth_format(VkPhysicalDevice device);
    static bool has_stencil_component(VkFormat format);

    static void copy_buffer_to_image(VkCommandBuffer command_buffer,
                                     VkBuffer buffer, VkImage image,
                                     uint32_t width, uint32_t height);
    static void copy_image_to_buffer(VkCommandBuffer command_buffer,
                                     VkImage image, uint32_t width,
                                     uint32_t height,
                                     VkImageLayout image_layout,
                                     VkBuffer buffer);
    static void transition_image_layout(const TransitionImageLayoutSpec& spec);

    static void cmd_begin_rendering_khr(VkInstance instance,
                                        VkCommandBuffer command_buffer,
                                        VkRenderingInfoKHR* render_info);
    static void cmd_end_rendering_khr(VkInstance instance,
                                      VkCommandBuffer command_buffer);
};
} // namespace Helios
