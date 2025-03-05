#include "AssetsBrowser.h"

#include "Helios/Core/Application.h"
#include "vulkan/vulkan_core.h"
#include <imgui.h>
#include <imgui_impl_vulkan.h>

#include <filesystem>

using directory_iterator = std::filesystem::directory_iterator;
namespace fs = std::filesystem;

constexpr int k_max_traverse_count = 30;
constexpr float k_min_directory_tree_width = 90.0f;

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
    texture = Texture::create(path);

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

    m_current_directory = &m_root_node;
    m_directory_tree_width = k_min_directory_tree_width;
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
                .icon = m_directory_icon_handle,
            });
            auto& new_dir = node.files.back();

            traverse_directory(new_dir);
        } else {
            FileType type;
            VkDescriptorSet icon = VK_NULL_HANDLE;

            std::string extension = dir_entry.path().extension().string();
            if (extension == ".lua") {
                type = FileType::Script;
                icon = m_script_icon_handle;
            } else if (extension == ".mat") {
                type = FileType::Material;
                icon = m_material_icon_handle;
            } else if (extension == ".scene") {
                type = FileType::Scene;
                icon = m_scene_icon_handle;
            } else if (extension == ".proj") {
                type = FileType::Project;
                icon = m_project_icon_handle;
            } else {
                type = FileType::Other;
                icon = m_file_icon_handle;
            }

            node.files.push_back(FileNode{
                .type = type,
                .path = dir_entry.path(),
                .icon = icon,
            });
        }
    }
    m_traverse_count++;
}

void AssetsBrowser::draw_icons(FileNode* directory, const glm::vec2& icon_size,
                               float padding) {
    
    if (!directory || directory->type != FileType::Directory) {
        return;
    }

    ImGui::BeginChild("FileBox");
    {
        float windowWidth =
            ImGui::GetContentRegionAvail().x; // Available width in the window
        float xCursor = 0.0f; // Tracks current X position on the row


        for (auto& file : directory->files) {

            if (xCursor + icon_size.x > windowWidth) {
                ImGui::NewLine(); // Move to the next row
                xCursor = 0.0f;   // Reset row position
            }

            ImGui::SameLine(0, padding);

            ImGui::BeginGroup(); 
            {
                ImGui::Image(file.icon, {icon_size.x, icon_size.y},
                             ImVec2{0, 1}, ImVec2{1, 0}); // Display image
                ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + icon_size.x); // Set the wrap position for text
                ImGui::TextWrapped(
                    "%s",
                    file.path.filename().string().c_str()); // Display text
                ImGui::PopTextWrapPos(); // Reset text wrap position after text
            }
            ImGui::EndGroup();

            xCursor += icon_size.x + padding; // Update X position
        }
    }

    ImGui::EndChild();       
}

void AssetsBrowser::draw_directory_tree(FileNode* root_directory, float width) {
    if (!root_directory || root_directory->type != FileType::Directory) {
        return;
    }

    ImGui::BeginChild("Directory tree", {width, 0});
    {
        draw_subdirectory(root_directory);
    }
    ImGui::EndChild();
}

void AssetsBrowser::draw_subdirectory(FileNode* directory) {
    auto filename = directory->path.filename();
    if (filename.string().empty()) {
        filename = directory->path.parent_path().filename();
    }
    ImGuiTreeNodeFlags node_flags =
        ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

    if (ImGui::TreeNodeEx(filename.string().c_str(), node_flags)) {
        for (auto& file : directory->files) {
            if (file.type == FileType::Directory) {
                draw_subdirectory(&file);
            }
        }
        ImGui::TreePop();
    }
}

void AssetsBrowser::draw_divider() {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.2f, 0.2f, 0.2f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.3f, 0.3f, 0.3f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.3f, 0.3f, 0.3f, 1.0f});
    ImGui::Button("##Divider", ImVec2(5.0f, ImGui::GetWindowHeight()));
    ImGui::PopStyleColor(3); 
    if (ImGui::IsItemActive()) {
        m_directory_tree_width += ImGui::GetIO().MouseDelta.x;
        m_directory_tree_width = std::clamp(
            m_directory_tree_width, k_min_directory_tree_width,
            ImGui::GetWindowWidth() - k_min_directory_tree_width -
                60.0f); // include extra padding for scrollbar and such
    }
    if (ImGui::IsItemActive() || ImGui::IsItemHovered()) {
        ImGui::SetMouseCursor(
            ImGuiMouseCursor_ResizeEW); 
    } else {
        ImGui::SetMouseCursor(
            ImGuiMouseCursor_Arrow); 
    }

}

void AssetsBrowser::on_update() {

    if (!m_project) {
        return;
    }
    ImGui::Begin("Assets Browser");
    {
        if (m_current_directory)
        {
            draw_directory_tree(&m_root_node, m_directory_tree_width);
            ImGui::SameLine();
            draw_divider();
            ImGui::SameLine();
            draw_icons(m_current_directory, {60, 60}, 15.0f);
        }
     
    }
    ImGui::End();
}
} 
