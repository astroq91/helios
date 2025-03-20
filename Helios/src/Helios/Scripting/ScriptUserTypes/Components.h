#pragma once
#include "Helios/ECSComponents/Components.h"
#include "Helios/Scene/Entity.h"
#include "MeshRenderer.h"
namespace Helios::ScriptUserTypes {

class ScriptComponents {
  public:
    ScriptComponents(Entity entity) : m_entity(entity) {}

    /* Getters */

    TransformComponent* get_transform() {
        return m_entity.try_get_component<TransformComponent>();
    }

    NameComponent* get_name() {
        return m_entity.try_get_component<NameComponent>();
    }

    CameraComponent* get_camera() {
        return m_entity.try_get_component<CameraComponent>();
    }

    DirectionalLightComponent* get_directional_light() {
        return m_entity.try_get_component<DirectionalLightComponent>();
    }

    PointLightComponent* get_point_light() {
        return m_entity.try_get_component<PointLightComponent>();
    }

    RigidBodyComponent* get_rigid_body() {
        return m_entity.try_get_component<RigidBodyComponent>();
    }

    ScriptMeshRenderer get_mesh_renderer() {
        return ScriptMeshRenderer(
            m_entity.try_get_component<MeshRendererComponent>());
    }

    /* Adders */

    TransformComponent* add_transform() {
        return &m_entity.add_component<TransformComponent>();
    }

    CameraComponent* add_camera() {
        return &m_entity.add_component<CameraComponent>();
    }

    DirectionalLightComponent* add_directional_light() {
        return &m_entity.add_component<DirectionalLightComponent>();
    }

    PointLightComponent* add_point_light() {
        return &m_entity.add_component<PointLightComponent>();
    }

    RigidBodyComponent* add_rigid_body() {
        return &m_entity.add_component<RigidBodyComponent>();
    }

    ScriptMeshRenderer add_mesh_renderer() {
        return ScriptMeshRenderer(
            &m_entity.add_component<MeshRendererComponent>());
    }

  private:
    Entity m_entity;
};

} // namespace Helios::ScriptUserTypes
