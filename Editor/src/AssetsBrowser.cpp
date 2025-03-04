#include "AssetsBrowser.h"
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include "Helios/Core/Application.h"

#include <filesystem>

using directory_iterator =
    std::filesystem::directory_iterator;
namespace fs = std::filesystem;

constexpr int k_max_traverse_count = 30;

namespace Helios {



void AssetsBrowser::init() { 
    m_icon_sampler = TextureSampler::create();
    m_file_icon_texture =
        Texture::create(RESOURCES_PATH "images/file_icon.png");
    m_folder_icon_texture =
        Texture::create(RESOURCES_PATH "images/folder_icon.png");

    auto& renderer = Application::get().get_renderer();
    auto& vulkan_context =
        Application::get().get_vulkan_manager()->get_context();

    VkCommandBuffer command_buffer = VulkanUtils::begin_single_time_commands(
        vulkan_context.device, vulkan_context.command_pool);
        
    VulkanUtils::transition_image_layout(
        {.image =
             m_file_icon_texture->get_image()->get_vk_image(),
         .old_layout = VK_IMAGE_LAYOUT_UNDEFINED,
         .new_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
         .dst_access_mask = VK_ACCESS_SHADER_READ_BIT,
         .src_stage_mask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
         .dst_stage_mask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
         .command_buffer = command_buffer});
     VulkanUtils::transition_image_layout(
        {.image = m_folder_icon_texture->get_image()->get_vk_image(),
         .old_layout = VK_IMAGE_LAYOUT_UNDEFINED,
         .new_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
         .dst_access_mask = VK_ACCESS_SHADER_READ_BIT,
         .src_stage_mask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
         .dst_stage_mask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
         .command_buffer = command_buffer});

     VulkanUtils::end_single_time_commands(
         command_buffer, vulkan_context.device, vulkan_context.command_pool,
         vulkan_context.graphics_queue);

     m_file_icon_handle = ImGui_ImplVulkan_AddTexture(
         m_icon_sampler->get_vk_sampler(),
         m_file_icon_texture->get_image()->get_vk_image_view(),
         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
     m_folder_icon_handle = ImGui_ImplVulkan_AddTexture(
         m_icon_sampler->get_vk_sampler(),
         m_folder_icon_texture->get_image()->get_vk_image_view(),
         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void AssetsBrowser::set_project(Project* project) {
    m_project = project;

    m_root_node = FileNode { 
        .type = FileType::Directory,
        .path = project->get_project_path()
    };

    traverse_directory(m_root_node);
    m_traverse_count = 0;
}

void AssetsBrowser::traverse_directory(FileNode& node) {
    if (m_traverse_count > k_max_traverse_count) { 
        return;
    }
    for (const auto& dir_entry : directory_iterator(node.path)) {
        if (dir_entry.is_directory()) {
            node.files.push_back(
                FileNode{
                    .type = FileType::Directory,
                    .path = dir_entry.path()
                });
            auto& new_dir = node.files.back();

            traverse_directory(new_dir);
        } else {
            FileType type;

            std::string extension = dir_entry.path().extension().string();
            if (extension == ".lua") {
                type = FileType::Script;
            } else if (extension == ".mat") {
                type = FileType::Material;
            } else {
                type = FileType::Other;
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
        ImGui::Image(m_folder_icon_handle, ImVec2{75, 75}, ImVec2{0, 1},
                     ImVec2{1, 0});
    }
    ImGui::End();


}
}