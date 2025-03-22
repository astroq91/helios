#pragma once
namespace Helios {
class MeshRendererComponent;
}
namespace Helios::ScriptUserTypes {
class ScriptMeshRenderer {
  public:
    ScriptMeshRenderer(MeshRendererComponent* component)
        : m_component(component) {}

    void load_mesh(const std::string& path);
    void load_material(const std::string& path);

    MeshRendererComponent* get_component() { return m_component; }

  private:
    MeshRendererComponent* m_component;
};
} // namespace Helios::ScriptUserTypes
