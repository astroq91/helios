#pragma once
#include "Helios/Scene/Entity.h"
#include "Helios/Scripting/ScriptUserTypes/PhysicsBody.h"
#include "MeshRenderer.h"
namespace Helios {
class TransformComponent;
class NameComponent;
class CameraComponent;
class DirectionalLightComponent;
class PointLightComponent;
} // namespace Helios
namespace Helios::ScriptUserTypes {

class ScriptComponents {
  public:
    ScriptComponents() {}
    ScriptComponents(Entity entity) : m_entity(entity) {}

    /* Getters */
    TransformComponent* get_transform();
    NameComponent* get_name();
    CameraComponent* get_camera();
    DirectionalLightComponent* get_directional_light();
    PointLightComponent* get_point_light();
    ScriptPhysicsBody get_physics_body();
    ScriptMeshRenderer get_mesh_renderer();

    /* Adders */
    TransformComponent* add_transform();
    CameraComponent* add_camera();
    DirectionalLightComponent* add_directional_light();
    PointLightComponent* add_point_light();
    ScriptPhysicsBody add_physics_body();
    ScriptMeshRenderer add_mesh_renderer();

  private:
    Entity m_entity;
};

} // namespace Helios::ScriptUserTypes
