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
    static Ref<Material> create(const std::filesystem::path& path) {
        auto material = make_ref<Material>();
        material->init_asset(path.string());
        material->init(path);
        return material;
    }

    const Ref<Texture>& get_diffuse() const { return m_diffuse; }
    const Ref<Texture>& get_specular() const { return m_specular; }
    const Ref<Texture>& get_emission() const { return m_emission; }
    float get_shininess() const { return m_shininess; }

    const Ref<Shader>& get_vertex_shader() const { return m_vertex_shader; }
    const Ref<Shader>& get_fragment_shader() const { return m_fragment_shader; }

  private:
    void init(const std::filesystem::path& path);

  private:
    Ref<Texture> m_diffuse = nullptr;
    Ref<Texture> m_specular = nullptr;
    Ref<Texture> m_emission = nullptr;
    Ref<Shader> m_vertex_shader = nullptr;
    Ref<Shader> m_fragment_shader = nullptr;
    float m_shininess = 32.0f;
};
} // namespace Helios
