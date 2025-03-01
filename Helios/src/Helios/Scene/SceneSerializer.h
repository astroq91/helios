#pragma once

#include <filesystem>
#include "Scene.h"

namespace Helios {
class SceneSerializer {
  public:
    SceneSerializer(Scene* scene) : m_scene(scene) {}

    void serialize(const std::filesystem::path& path);
    void deserialize(const std::filesystem::path& path);

  private:
    Scene* m_scene;
};
} // namespace Helios
