#pragma once

#include <filesystem>
#include "Scene.h"

namespace Helios {
class SceneSerializer {
  public:
    SceneSerializer(Scene* scene) : m_scene(scene) {}

    void serialize_to_path(const std::filesystem::path& path);
    void deserialize_from_path(const std::filesystem::path& path);

    void serialize_to_string(std::string& buffer);
    void deserialize_from_string(const std::string& buffer);

  private:
    Scene* m_scene;
};
} // namespace Helios
