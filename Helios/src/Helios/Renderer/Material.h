#pragma once
#include "Helios/Assets/Asset.h"
#include "Helios/Core/Core.h"
#include "Texture.h"
#include <glm/glm.hpp>

namespace Helios {
class Material : public Asset {
  public:
    /**
     * Create a new material.
     * @param path The relative path to the material file
     */
    static Ref<Material> create(const std::string& path) {
        auto material = make_ref<Material>();
        material->init_asset(path);
        material->init(path);
        return material;
    }

    const Ref<Texture>& get_diffuse() const { return m_diffuse; }
    const Ref<Texture>& get_specular() const { return m_specular; }
    const Ref<Texture>& get_emission() const { return m_emission; }
    float get_shininess() const { return m_shininess; }

  private:
    void init(const std::string& path);

  private:
    Ref<Texture> m_diffuse = nullptr;
    Ref<Texture> m_specular = nullptr;
    Ref<Texture> m_emission = nullptr;
    float m_shininess = 32.0f;
};
} // namespace Helios
