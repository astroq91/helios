#pragma once
#include "Helios/Core/Application.h"
#include "Helios/ECSComponents/Components.h"
namespace Helios::ScriptUserTypes {
class ScriptMeshRenderer {
  public:
    ScriptMeshRenderer(MeshRendererComponent* component)
        : m_component(component) {}

    void load_mesh(const std::string& path) {
        if (path == "Cube") {
            m_component->mesh =
                Application::get().get_asset_manager().get_mesh("Cube");
        } else {
            m_component->mesh = Mesh::create(path);
        }
    }
    void load_material(const std::string& path) {
        m_component->material = Material::create(path);
    }

    MeshRendererComponent* get_component() { return m_component; }

  private:
    MeshRendererComponent* m_component;
};
} // namespace Helios::ScriptUserTypes
