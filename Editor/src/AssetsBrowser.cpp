#include "AssetsBrowser.h"

#include "Helios/Core/Application.h"
#include "imgui_internal.h"
#include "vulkan/vulkan_core.h"
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <map>

#include <filesystem>

#ifdef _WINDOWS
#include <shellapi.h>
#endif
#ifdef _LINUX
#include <cstdlib>
#endif

using directory_iterator = std::filesystem::directory_iterator;
namespace fs = std::filesystem;

constexpr int k_max_traverse_count = 30;
constexpr float k_min_directory_tree_width = 90.0f;
constexpr ImVec2 k_icon_size = {60, 60};
constexpr ImVec2 k_drag_drop_icon_size = {40, 40};
constexpr float k_icon_padding = 15.0f;

const std::unordered_map<Helios::FileType, const char*>
    k_file_types_payload_identifiers{
        {Helios::FileType::Script, "PAYLOAD_SCRIPT"},
        {Helios::FileType::Material, "PAYLOAD_MATERIAL"},
        {Helios::FileType::Mesh, "PAYLOAD_MESH"},
    };

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
                              SharedPtr<Texture>& texture,
                              VkDescriptorSet& handle) {
    texture = Texture::create(path);

    VulkanUtils::transition_image_layout({
        .image = texture->get_image()->get_vk_image(),
        .old_layout = VK_IMAGE_LAYOUT_UNDEFINED,
        .new_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        .dst_access_mask = VK_ACCESS_SHADER_READ_BIT,
        .src_stage_mask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        .dst_stage_mask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        .command_buffer = command_buffer,
    });
    handle =
        ImGui_ImplVulkan_AddTexture(m_icon_sampler->get_vk_sampler(),
                                    texture->get_image()->get_vk_image_view(),
                                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void AssetsBrowser::set_project(Project* project) {
    m_project = project;

    m_root_node = FileNode{.type = FileType::Directory,
                           .path = project->get_project_path()};

    recreate_directory_tree();
    m_directory_tree_width = k_min_directory_tree_width;
}

void AssetsBrowser::traverse_directory(
    FileNode& node, std::optional<fs::path> set_current_directory) {
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

            if (set_current_directory) {
                if (dir_entry.path() == set_current_directory.value()) {
                    m_current_directory = &node.files[node.files.size() - 1];
                }
            }

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
            } else if (extension == ".obj") {
                type = FileType::Mesh;
                icon = m_file_icon_handle;
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

void AssetsBrowser::recreate_directory_tree() {
    std::optional<fs::path> current_path;
    if (m_current_directory && fs::exists(m_current_directory->path)) {
        current_path = m_current_directory->path;
    }

    m_current_directory = nullptr;
    m_copy_file_buffer = nullptr;
    m_root_node.files.clear();
    traverse_directory(m_root_node, current_path);
    m_traverse_count = 0;

    if (!m_current_directory) {
        m_current_directory = &m_root_node;
    }
}

std::optional<fs::path> construct_new_path(const fs::path& src,
                                           const fs::path& dst) {
    std::optional<fs::path> ret;
    std::string new_name = src.stem().string() + src.extension().string();
    fs::path new_path = dst / new_name;
    if (fs::exists(new_path)) {
        // Create a new name if the filename is already taken

        const int k_max_new_name_checks = 1000;
        for (int i = 0; i < k_max_new_name_checks; i++) {
            std::string new_name = src.stem().string() + " - " +
                                   std::to_string(i) + src.extension().string();
            fs::path new_path = dst / new_name;
            if (!fs::exists(new_path)) {
                ret = new_path;
                break;
            }
        }
    } else {
        // If it doesnt extist, just use the same filename
        ret = new_path;
    }
    return ret;
}

void AssetsBrowser::draw_file_box(FileNode* directory) {

    if (!directory || directory->type != FileType::Directory) {
        return;
    }

    ImGui::BeginChild("FileBox");
    {
        if (ImGui::IsWindowHovered() &&
            ImGui::IsMouseClicked(ImGuiMouseButton_Right) &&
            !ImGui::IsAnyItemHovered()) {
            ImGui::OpenPopup("Assets actions");
        }

        if (ImGui::BeginPopup("Assets actions")) {
            ImGui::BeginDisabled(m_copy_file_buffer == nullptr);
            if (ImGui::MenuItem("Paste")) {

                // Try to find a new name
                auto new_path = construct_new_path(m_copy_file_buffer->path,
                                                   directory->path);
                if (new_path) {
                    fs::copy(m_copy_file_buffer->path, new_path.value(),
                             std::filesystem::copy_options::recursive);
                    recreate_directory_tree();
                }
            }
            ImGui::EndDisabled();
            ImGui::Separator();
            if (ImGui::MenuItem("Open in file browser")) {
#ifdef _WINDOWS
                ShellExecuteA(nullptr, "open",
                              m_current_directory->path.string().c_str(),
                              nullptr, nullptr, SW_SHOWDEFAULT);
#endif
#ifdef _LINUX
                std::string cmd =
                    "xdg-open " + m_current_directory->path.string();
                std::system(cmd.c_str());
#endif
            }
            if (ImGui::MenuItem("Refresh files")) {
                recreate_directory_tree();
            }
            ImGui::EndPopup();
        }

        // Draw the icons
        draw_file_icons(directory);
    }
    ImGui::EndChild();
}

void AssetsBrowser::draw_file_icons(FileNode* directory) {

    float windowWidth =
        ImGui::GetContentRegionAvail().x; // Available width in the window
    float xCursor = 0.0f; // Tracks current X position on the row

    std::vector<FileNode*> files_to_delete;
    for (auto& file : directory->files) {

        if (xCursor + k_icon_size.x > windowWidth) {
            ImGui::NewLine(); // Move to the next row
            xCursor = 0.0f;   // Reset row position
        }

        ImGui::SameLine(0, k_icon_padding);

        ImGui::BeginGroup();
        {
            ImGui::PushID(file.path.string().c_str());
            ImGui::PushStyleColor(ImGuiCol_Button,
                                  ImVec4(0, 0, 0, 0)); // No button color
            ImGui::PushStyleColor(
                ImGuiCol_ButtonHovered,
                ImVec4(0.2f, 0.2f, 0.2f, 1.0f)); // No hover color
            ImGui::PushStyleColor(
                ImGuiCol_ButtonActive,
                ImVec4(0.2f, 0.2f, 0.2f, 1.0f)); // No active color
            ImGui::ImageButton(file.icon, k_icon_size, ImVec2{0, 1},
                               ImVec2{1, 0});
            ImGui::PopStyleColor(3); // Restore previous colors

            if (file.draggable() &&
                k_file_types_payload_identifiers.contains(file.type)) {
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                    ImGui::SetDragDropPayload(
                        k_file_types_payload_identifiers.at(file.type),
                        &file.path, sizeof(fs::path));
                    ImGui::Image(file.icon, k_drag_drop_icon_size);
                    ImGui::EndDragDropSource();
                }
            }

            if (ImGui::IsItemHovered() &&
                ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
                ImGui::OpenPopup("File action");
            }

            if (ImGui::BeginPopup("File action")) {
                if (ImGui::MenuItem("Copy")) {
                    m_copy_file_buffer = &file;
                }
                if (ImGui::MenuItem("Delete")) {
                    files_to_delete.push_back(&file);
                }
                ImGui::EndPopup();
            }

            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                if (file.type == FileType::Directory) {
                    m_current_directory = &file;
                    m_open_selected_directory = true;
                }
                handle_icon_double_click(&file);
            }

            ImGui::PushTextWrapPos(
                ImGui::GetCursorPos().x +
                k_icon_size.x); // Set the wrap position for text
            ImGui::TextWrapped(
                "%s",
                file.path.filename().string().c_str()); // Display text
            ImGui::PopTextWrapPos(); // Reset text wrap position after text

            ImGui::PopID();
        }
        ImGui::EndGroup();

        xCursor += k_icon_size.x + k_icon_padding; // Update X position
    }

    if (!files_to_delete.empty()) {
        for (auto file : files_to_delete) {
            fs::remove_all(file->path);
        }
        recreate_directory_tree();
    }
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
    ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_SpanAvailWidth |
                                    ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                    ImGuiTreeNodeFlags_OpenOnArrow;
    if (m_current_directory == directory) {
        node_flags |= ImGuiTreeNodeFlags_Selected;
        if (m_open_selected_directory) {
            ImGui::SetNextItemOpen(true);
            m_open_selected_directory = false;
        }
    }

    if (directory->path == m_root_node.path) {
        node_flags |= ImGuiTreeNodeFlags_DefaultOpen;
    }

    ImVec2 node_start_pos = ImGui::GetCursorScreenPos();
    bool node_open = ImGui::TreeNodeEx(filename.string().c_str(), node_flags);

    ImVec2 node_end_pos = ImVec2(node_start_pos.x + ImGui::GetItemRectSize().x,
                                 node_start_pos.y + ImGui::GetItemRectSize().y);
    ImVec2 mouse_pos = ImGui::GetMousePos();

    bool clicked_inside_node =
        (mouse_pos.x > node_start_pos.x && mouse_pos.x < node_end_pos.x &&
         mouse_pos.y > node_start_pos.y && mouse_pos.y < node_end_pos.y);

    if (ImGui::IsItemClicked() && clicked_inside_node) {
        m_current_directory = directory; // Select when clicking label only
    }

    if (node_open) {
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
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                          ImVec4{0.3f, 0.3f, 0.3f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                          ImVec4{0.3f, 0.3f, 0.3f, 1.0f});
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
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
    } else {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
    }
}

void AssetsBrowser::handle_icon_double_click(const FileNode* file) const {
    switch (file->type) {
    case FileType::Scene: {
        if (m_scene_selected_callback) {
            m_scene_selected_callback(file->path);
        }
        break;
    }
    case FileType::Script:
    case FileType::Material:
    case FileType::Project:
    case FileType::Other: {
#ifdef _WINDOWS
        ShellExecuteA(nullptr, "open", file->path.string().c_str(), nullptr,
                      nullptr, SW_SHOWNORMAL);
#endif
#ifdef _LINUX
        std::string cmd = "xdg-open " + file->path.string();
        std::system(cmd.c_str());
#endif
        break;
    }
    default:;
    }
}

void AssetsBrowser::on_update() {

    if (!m_project) {
        return;
    }
    ImGui::Begin("Assets Browser");
    {
        if (m_current_directory) {
            draw_directory_tree(&m_root_node, m_directory_tree_width);
            ImGui::SameLine();
            draw_divider();
            ImGui::SameLine();
            draw_file_box(m_current_directory);
        }
    }
    ImGui::End();
}
} // namespace Helios
