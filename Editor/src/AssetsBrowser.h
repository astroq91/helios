#pragma once
#include <entt/entt.hpp>

#include "Helios/Scene/Scene.h"
#include "Helios/Core/Core.h"
#include "Helios/Scene/Entity.h"
#include "Project.h"

namespace Helios {
enum class FileType {
    Script,
    Material,
    Directory, 
    Other
};

struct FileNode {
  public:
    FileType type;
    std::filesystem::path path;
    std::vector<FileNode> files;
};

class AssetsBrowser {
  public:
    void init();
    void on_update();

    void set_project(Project* project);
    void traverse_directory(FileNode& node);

  private:
    Project* m_project = nullptr;

    Ref<TextureSampler> m_icon_sampler;
    Ref<Texture> m_file_icon_texture;
    Ref<Texture> m_folder_icon_texture;
    VkDescriptorSet m_file_icon_handle;
    VkDescriptorSet m_folder_icon_handle;

    FileNode m_root_node;
    int m_traverse_count = 0;
};
} // namespace Helios
