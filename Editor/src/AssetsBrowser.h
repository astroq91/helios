#pragma once
#include <entt/entt.hpp>

#include "Helios/Core/Core.h"
#include "Helios/Renderer/Texture.h"
#include "Helios/Renderer/TextureSampler.h"
#include "Project.h"
#include <volk/volk.h>
#include <glm/glm.hpp>

namespace Helios {
enum class FileType { Script, Material, Mesh, Scene, Project, Directory, Other };

struct FileNode {
    FileType type;
    std::filesystem::path path;
    VkDescriptorSet icon = VK_NULL_HANDLE;
    std::vector<FileNode> files;

    bool draggable() const {
        return type == FileType::Script || type == FileType::Material || type == FileType::Mesh;
    }
};

class AssetsBrowser {
  public:
    void init();

    void on_update();

    void set_project(Project* project);

  private:
    void init_icon(const std::filesystem::path& path,
                   VkCommandBuffer command_buffer, Ref<Texture>& texture,
                   VkDescriptorSet& handle);
    void traverse_directory(FileNode& node);
    void draw_icons(FileNode* directory);
    void draw_directory_tree(FileNode* root_directory, float width);
    void draw_subdirectory(FileNode* directory);
    void draw_divider();

    void handle_icon_click(FileNode* file);

  private:
    Project* m_project = nullptr;

    Ref<TextureSampler> m_icon_sampler;

    Ref<Texture> m_file_icon_texture;
    Ref<Texture> m_directory_icon_texture;
    Ref<Texture> m_scene_icon_texture;
    Ref<Texture> m_script_icon_texture;
    Ref<Texture> m_material_icon_texture;
    Ref<Texture> m_project_icon_texture;

    VkDescriptorSet m_file_icon_handle;
    VkDescriptorSet m_directory_icon_handle;
    VkDescriptorSet m_scene_icon_handle;
    VkDescriptorSet m_script_icon_handle;
    VkDescriptorSet m_material_icon_handle;
    VkDescriptorSet m_project_icon_handle;

    FileNode m_root_node;
    FileNode* m_current_directory = nullptr;
    int m_traverse_count = 0;

    float m_directory_tree_width = 0;
    bool m_open_selected_directory = false;

};
} // namespace Helios
