#pragma once
#include <entt/entt.hpp>

#include "Helios/Core/Core.h"
#include "Helios/Renderer/Texture.h"
#include "Helios/Renderer/TextureSampler.h"
#include "Project.h"
#include <glm/glm.hpp>
#include <volk/volk.h>

namespace Helios {
enum class FileType {
    Script,
    Material,
    Mesh,
    Scene,
    Project,
    Directory,
    Other
};

struct FileNode {
    FileType type;
    std::filesystem::path path;
    VkDescriptorSet icon = VK_NULL_HANDLE;
    std::vector<FileNode> files;

    bool draggable() const {
        return type == FileType::Script || type == FileType::Material ||
               type == FileType::Mesh || type == FileType::Scene;
    }
};

class AssetsBrowser {
  public:
    void init();

    void on_update();

    void set_project(Project* project);
    void set_on_scene_selected_callback(
        const std::function<void(const std::filesystem::path&)>& callback) {
        m_scene_selected_callback = callback;
    }

  private:
    void init_icon(const std::filesystem::path& path,
                   VkCommandBuffer command_buffer, SharedPtr<Texture>& texture,
                   VkDescriptorSet& handle);
    void traverse_directory(FileNode& node,
                            std::optional<std::filesystem::path>
                                set_current_directory = std::nullopt);
    void recreate_directory_tree();
    void draw_file_box(FileNode* directory);
    void draw_file_icons(FileNode* directory);
    void draw_directory_tree(FileNode* root_directory, float width);
    void draw_subdirectory(FileNode* directory);
    void draw_divider();

    void handle_icon_double_click(const FileNode* file) const;
    void handle_icon_right_click(const FileNode* file) const;

  private:
    Project* m_project = nullptr;

    SharedPtr<TextureSampler> m_icon_sampler;

    SharedPtr<Texture> m_file_icon_texture;
    SharedPtr<Texture> m_directory_icon_texture;
    SharedPtr<Texture> m_scene_icon_texture;
    SharedPtr<Texture> m_script_icon_texture;
    SharedPtr<Texture> m_material_icon_texture;
    SharedPtr<Texture> m_project_icon_texture;

    VkDescriptorSet m_file_icon_handle;
    VkDescriptorSet m_directory_icon_handle;
    VkDescriptorSet m_scene_icon_handle;
    VkDescriptorSet m_script_icon_handle;
    VkDescriptorSet m_material_icon_handle;
    VkDescriptorSet m_project_icon_handle;

    std::function<void(const std::filesystem::path&)>
        m_scene_selected_callback = nullptr;

    FileNode m_root_node;
    FileNode* m_current_directory = nullptr;
    int m_traverse_count = 0;

    float m_directory_tree_width = 0;
    bool m_open_selected_directory = false;

    FileNode* m_copy_file_buffer = nullptr;
};
} // namespace Helios
