#pragma once

#include "Scene.h"
#include <filesystem>
#include <yaml-cpp/yaml.h>

namespace Helios {
class SceneSerializer {
  public:
    SceneSerializer(Scene* scene) : m_scene(scene) {}

    void serialize_to_path(const std::filesystem::path& path);
    void deserialize_from_path(const std::filesystem::path& path);

    void serialize_to_string(std::string& buffer);
    void deserialize_from_string(const std::string& buffer);

  private:
    void deserialize_from_string_with_parent(const YAML::Node& entities,
                                             Entity parent);

    void serialize_entity(YAML::Emitter& out, Entity entity);

    void serialize_entity_components(YAML::Emitter& out, Entity entity);

  private:
    Scene* m_scene;
};
} // namespace Helios
