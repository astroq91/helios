#include "Texture.h"

#include <stb_image.h>

#include "Buffer.h"
#include "Helios/Core/Application.h"
#include "Helios/Core/IOUtils.h"
#include "Renderer.h"

namespace Helios {
Texture::~Texture() {
    Renderer& renderer = Application::get().get_renderer();
    renderer.deregister_texture(m_texture_index);
}

void Texture::init(const std::filesystem::path& path, VkFormat format) {
    const VulkanContext& context =
        Application::get().get_vulkan_manager()->get_context();
    Renderer& renderer = Application::get().get_renderer();

    m_name = path.string();

    if (path.empty()) {
        return;
    }

    int tex_width, tex_height, tex_channels;
    stbi_uc* pixles = stbi_load(IOUtils::resolve_path(
                      Application::get().get_asset_base_path(), path)
                      .string().c_str(),
        &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
    VkDeviceSize image_size = tex_width * tex_height * 4;

    if (!pixles) {
        HL_ERROR("Failed to load texture image: {0}", path.string());
    }

    std::unique_ptr<Buffer> staging_buffer =
        Buffer::create_unique(image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* data;
    vkMapMemory(context.device, staging_buffer->get_vk_memory(), 0, image_size,
                0, &data);
    memcpy(data, pixles, static_cast<size_t>(image_size));
    vkUnmapMemory(context.device, staging_buffer->get_vk_memory());

    stbi_image_free(pixles);

    m_image = Image::create({
        .width = static_cast<uint32_t>(tex_width),
        .height = static_cast<uint32_t>(tex_height),
        .format = format,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .memory_property = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    });

    VkCommandBuffer command_buffer = VulkanUtils::begin_single_time_commands(
        context.device, context.command_pool);

    VulkanUtils::transition_image_layout(
        {.image = m_image->get_vk_image(),
         .old_layout = VK_IMAGE_LAYOUT_UNDEFINED,
         .new_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
         .src_access_mask = 0,
         .dst_access_mask = VK_ACCESS_TRANSFER_WRITE_BIT,
         .src_stage_mask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
         .dst_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT,
         .command_buffer = command_buffer});
    VulkanUtils::copy_buffer_to_image(
        command_buffer, staging_buffer->get_vk_buffer(),
        m_image->get_vk_image(), static_cast<uint32_t>(tex_width),
        static_cast<uint32_t>(tex_height));
    VulkanUtils::transition_image_layout(
        {.image = m_image->get_vk_image(),
         .old_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
         .new_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
         .src_access_mask = VK_ACCESS_TRANSFER_WRITE_BIT,
         .dst_access_mask = VK_ACCESS_SHADER_READ_BIT,
         .src_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT,
         .dst_stage_mask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
         .command_buffer = command_buffer});

    VulkanUtils::end_single_time_commands(command_buffer, context.device,
                                          context.command_pool,
                                          context.graphics_queue);

    m_texture_index = renderer.register_texture(*this);
}

void Texture::init(void* data, uint32_t width, uint32_t height, size_t size, VkFormat format) {
    const VulkanContext& context =
        Application::get().get_vulkan_manager()->get_context();
    Renderer& renderer = Application::get().get_renderer();

    std::unique_ptr<Buffer> staging_buffer =
        Buffer::create_unique(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* data_map;
    vkMapMemory(context.device, staging_buffer->get_vk_memory(), 0, size, 0,
                &data_map);
    memcpy(data_map, data, static_cast<size_t>(size));
    vkUnmapMemory(context.device, staging_buffer->get_vk_memory());

    m_image = Image::create({
        .width = width,
        .height = height,
        .format = format,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .memory_property = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    });

    VkCommandBuffer command_buffer = VulkanUtils::begin_single_time_commands(
        context.device, context.command_pool);

    VulkanUtils::transition_image_layout(
        {.image = m_image->get_vk_image(),
         .old_layout = VK_IMAGE_LAYOUT_UNDEFINED,
         .new_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
         .src_access_mask = 0,
         .dst_access_mask = VK_ACCESS_TRANSFER_WRITE_BIT,
         .src_stage_mask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
         .dst_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT,
         .command_buffer = command_buffer});
    VulkanUtils::copy_buffer_to_image(command_buffer,
                                      staging_buffer->get_vk_buffer(),
                                      m_image->get_vk_image(), width, height);
    VulkanUtils::transition_image_layout(
        {.image = m_image->get_vk_image(),
         .old_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
         .new_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
         .src_access_mask = VK_ACCESS_TRANSFER_WRITE_BIT,
         .dst_access_mask = VK_ACCESS_SHADER_READ_BIT,
         .src_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT,
         .dst_stage_mask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
         .command_buffer = command_buffer});

    VulkanUtils::end_single_time_commands(command_buffer, context.device,
                                          context.command_pool,
                                          context.graphics_queue);

    m_texture_index = renderer.register_texture(*this);
}
} // namespace Helios
