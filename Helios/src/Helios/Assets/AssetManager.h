#pragma once
#include "AssetLibrary.h"
#include "Helios/Renderer/Geometry.h"
#include "Helios/Renderer/Material.h"
#include "Helios/Renderer/Shader.h"
#include "Helios/Renderer/Texture.h"

namespace Helios {
class AssetManager {
  public:
    void init();

    void clear_assets();

    void add_texture(const Ref<Texture>& texture) {
        m_textures.add_asset(texture);
    }

    void add_geometry(const Ref<Geometry>& geometry) { m_geometries.add_asset(geometry); }

    void add_shader(const Ref<Shader>& shader) { m_shaders.add_asset(shader); }

    void add_material(const Ref<Material>& material) {
        m_materials.add_asset(material);
    }

    Ref<Texture> get_texture(const std::string& name) {
        return m_textures.get_asset(name);
    }

    Ref<Geometry> get_geometry(const std::string& name) {
        return m_geometries.get_asset(name);
    }

    Ref<Shader> get_shader(const std::string& name) {
        return m_shaders.get_asset(name);
    }

    Ref<Material> get_material(const std::string& name) {
        return m_materials.get_asset(name);
    }

  private:
    AssetLibrary<Texture> m_textures;
    AssetLibrary<Geometry> m_geometries;
    AssetLibrary<Shader> m_shaders;
    AssetLibrary<Material> m_materials;
};
} // namespace Helios
