#pragma once
#include "Helios/Assets/Asset.h"
#include "Helios/Core/Core.h"
#include "Helios/Renderer/Shader.h"
#include "Texture.h"
#include <filesystem>
#include <glm/glm.hpp>

namespace Helios {
class Material : public Asset {
  public:
    /**
     * Create a new material.
     * @param path The relative path to the material file
     */
    static SharedPtr<Material> create(const std::filesystem::path& path) {
        auto material = SharedPtr<Material>::create();
        material->init_asset(path.string());
        material->init(path);
        return material;
    }

    const SharedPtr<Texture>& get_diffuse() const { return m_diffuse; }
    const SharedPtr<Texture>& get_specular() const { return m_specular; }
    const SharedPtr<Texture>& get_emission() const { return m_emission; }
    float get_shininess() const { return m_shininess; }

    const SharedPtr<Shader>& get_vertex_shader() const { return m_vertex_shader; }
    const SharedPtr<Shader>& get_fragment_shader() const { return m_fragment_shader; }

  private:
    void init(const std::filesystem::path& path);

  private:
    SharedPtr<Texture> m_diffuse = nullptr;
    SharedPtr<Texture> m_specular = nullptr;
    SharedPtr<Texture> m_emission = nullptr;
    SharedPtr<Shader> m_vertex_shader = nullptr;
    SharedPtr<Shader> m_fragment_shader = nullptr;
    float m_shininess = 32.0f;
};
} // namespace Helios
