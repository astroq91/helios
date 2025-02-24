#pragma once

#include "Scene.h"

namespace Helios {
class SceneSerializer {
  public:
    SceneSerializer(Scene* scene) : m_scene(scene) {}

    void serialize(const std::string& path);
    void deserialize(const std::string& path);

  private:
    Scene* m_scene;
};
} // namespace Helios
