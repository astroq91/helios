#include "AssetsBrowser.h"
#include "Helios/Core/Application.h"
#include "vulkan/vulkan_core.h"
#include <imgui.h>
#include <imgui_impl_vulkan.h>

#include <filesystem>

using directory_iterator = std::filesystem::directory_iterator;
namespace fs = std::filesystem;

constexpr int k_max_traverse_count = 30;

namespace Helios {

void AssetsBrowser::init() {
    m_icon_sampler = TextureSampler::create();
    m_file_icon_texture =
        Texture::create(RESOURCES_PATH "images/file_icon.png");
    auto& renderer = Application::get().get_renderer();
    auto& vulkan_context =
        Application::get().get_vulkan_manager()->get_context();

    VkCommandBuffer command_buffer = VulkanUtils::begin_single_time_commands(
        vulkan_context.device, vulkan_context.command_pool);

    init_icon(RESOURCES_PATH "images/file_icon.png", command_buffer,
              m_file_icon_texture, m_file_icon_handle);

    init_icon(RESOURCES_PATH "images/directory_icon.png", command_buffer,
              m_directory_icon_texture, m_directory_icon_handle);

    init_icon(RESOURCES_PATH "images/script_icon.png", command_buffer,
              m_script_icon_texture, m_script_icon_handle);

    init_icon(RESOURCES_PATH "images/scene_icon.png", command_buffer,
              m_scene_icon_texture, m_scene_icon_handle);

    init_icon(RESOURCES_PATH "images/material_icon.png", command_buffer,
              m_material_icon_texture, m_material_icon_handle);

    init_icon(RESOURCES_PATH "images/project_icon.png", command_buffer,
              m_project_icon_texture, m_project_icon_handle);

    VulkanUtils::end_single_time_commands(command_buffer, vulkan_context.device,
                                          vulkan_context.command_pool,
                                          vulkan_context.graphics_queue);
}

void AssetsBrowser::init_icon(const fs::path& path,
                              VkCommandBuffer command_buffer,
                              Ref<Texture>& texture, VkDescriptorSet& handle) {
    texture = Texture::create(RESOURCES_PATH "images/folder_icon.png");

    VulkanUtils::transition_image_layout(
        {.image = texture->get_image()->get_vk_image(),
         .old_layout = VK_IMAGE_LAYOUT_UNDEFINED,
         .new_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
         .dst_access_mask = VK_ACCESS_SHADER_READ_BIT,
         .src_stage_mask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
         .dst_stage_mask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
         .command_buffer = command_buffer});
    handle =
        ImGui_ImplVulkan_AddTexture(m_icon_sampler->get_vk_sampler(),
                                    texture->get_image()->get_vk_image_view(),
                                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void AssetsBrowser::set_project(Project* project) {
    m_project = project;

    m_root_node = FileNode{.type = FileType::Directory,
                           .path = project->get_project_path()};

    traverse_directory(m_root_node);
    m_traverse_count = 0;
}

void AssetsBrowser::traverse_directory(FileNode& node) {
    if (m_traverse_count > k_max_traverse_count) {
        return;
    }
    for (const auto& dir_entry : directory_iterator(node.path)) {
        if (dir_entry.is_directory()) {
            node.files.push_back(FileNode{
                .type = FileType::Directory,
                .path = dir_entry.path(),
                .icon = m_directory_icon_texture,
            });
            auto& new_dir = node.files.back();

            traverse_directory(new_dir);
        } else {
            FileType type;
            Ref<Texture> icon = nullptr;

            std::string extension = dir_entry.path().extension().string();
            if (extension == ".lua") {
                type = FileType::Script;
                icon = m_script_icon_texture;
            } else if (extension == ".mat") {
                type = FileType::Material;
                icon = m_material_icon_texture;
            } else if (extension == ".scene") {
                type = FileType::Other;
                icon = m_scene_icon_texture;
            } else if (extension == ".proj") {
                type = FileType::Project;
                icon = m_project_icon_texture;
            } else {
                type = FileType::Other;
                icon = m_file_icon_texture;
            }

            node.files.push_back(FileNode{
                .type = type,
                .path = dir_entry.path(),
            });
        }
    }
    m_traverse_count++;
}

void AssetsBrowser::on_update() {
    if (!m_project) {
        return;
    }

    ImGui::Begin("Assets Browser");
    {
        ImGui::Image(m_file_icon_handle, ImVec2{75, 75}, ImVec2{0, 1},
                     ImVec2{1, 0});
    }
    ImGui::End();
}
} // namespace Helios
