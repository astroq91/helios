#include "Components.h"
#include "Helios/ECSComponents/Components.h"
#include "Helios/Scripting/ScriptUserTypes/RigidBody.h"

namespace Helios::ScriptUserTypes {

TransformComponent* ScriptComponents::get_transform() {
    return m_entity.try_get_component<TransformComponent>();
}

NameComponent* ScriptComponents::get_name() {
    return m_entity.try_get_component<NameComponent>();
}

CameraComponent* ScriptComponents::get_camera() {
    return m_entity.try_get_component<CameraComponent>();
}

DirectionalLightComponent* ScriptComponents::get_directional_light() {
    return m_entity.try_get_component<DirectionalLightComponent>();
}

PointLightComponent* ScriptComponents::get_point_light() {
    return m_entity.try_get_component<PointLightComponent>();
}

ScriptRigidBody ScriptComponents::get_rigid_body() {
    return ScriptRigidBody(m_entity.try_get_component<RigidBodyComponent>(),
                           m_entity);
}

ScriptMeshRenderer ScriptComponents::get_mesh_renderer() {
    return ScriptMeshRenderer(
        m_entity.try_get_component<MeshRendererComponent>());
}

/* Adders */

TransformComponent* ScriptComponents::add_transform() {
    return &m_entity.add_component<TransformComponent>();
}

CameraComponent* ScriptComponents::add_camera() {
    return &m_entity.add_component<CameraComponent>();
}

DirectionalLightComponent* ScriptComponents::add_directional_light() {
    return &m_entity.add_component<DirectionalLightComponent>();
}

PointLightComponent* ScriptComponents::add_point_light() {
    return &m_entity.add_component<PointLightComponent>();
}

ScriptRigidBody ScriptComponents::add_rigid_body() {
    return ScriptRigidBody(&m_entity.add_component<RigidBodyComponent>(),
                           m_entity);
}

ScriptMeshRenderer ScriptComponents::add_mesh_renderer() {
    return ScriptMeshRenderer(&m_entity.add_component<MeshRendererComponent>());
}
} // namespace Helios::ScriptUserTypes
