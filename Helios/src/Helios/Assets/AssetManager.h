#pragma once
#include "AssetLibrary.h"
#include "Helios/Renderer/Material.h"
#include "Helios/Renderer/Mesh.h"
#include "Helios/Renderer/Shader.h"
#include "Helios/Renderer/Texture.h"

namespace Helios {

class AssetManager {
  public:
    void init();

    void clear_assets();

    void add_texture(const SharedPtr<Texture>& texture) {
        m_textures.add_asset(texture);
    }

    void add_mesh(const SharedPtr<Mesh>& mesh) { m_meshes.add_asset(mesh); }

    void add_shader(const SharedPtr<Shader>& shader) {
        m_shaders.add_asset(shader);
    }

    void add_material(const SharedPtr<Material>& material) {
        m_materials.add_asset(material);
    }

    SharedPtr<Texture> get_texture(const std::string& name) {
        return m_textures.get_asset(name);
    }

    SharedPtr<Mesh> get_mesh(const std::string& name) {
        return m_meshes.get_asset(name);
    }

    SharedPtr<Shader> get_shader(const std::string& name) {
        return m_shaders.get_asset(name);
    }

    SharedPtr<Material> get_material(const std::string& name) {
        return m_materials.get_asset(name);
    }

  private:
    AssetLibrary<Texture> m_textures;
    AssetLibrary<Mesh> m_meshes;
    AssetLibrary<Shader> m_shaders;
    AssetLibrary<Material> m_materials;
};
} // namespace Helios
