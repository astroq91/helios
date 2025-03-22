#include "MeshRenderer.h"
#include "Helios/Core/Application.h"
#include "Helios/ECSComponents/Components.h"

namespace Helios::ScriptUserTypes {

void ScriptMeshRenderer::load_mesh(const std::string& path) {
    if (path == "Cube") {
        m_component->mesh =
            Application::get().get_asset_manager().get_mesh("Cube");
    } else {
        m_component->mesh = Mesh::create(path);
    }
}
void ScriptMeshRenderer::load_material(const std::string& path) {
    m_component->material = Material::create(path);
}
} // namespace Helios::ScriptUserTypes
