#include "RigidBody.h"
#include "Helios/Core/Application.h"
#include "Helios/ECSComponents/Components.h"

namespace Helios::ScriptUserTypes {

double ScriptRigidBody::get_mass() const { return m_component->mass; }
void ScriptRigidBody::set_mass(double value) {
    m_component->mass = value;
    m_entity.update_rigid_body_mass(value);
}

double ScriptRigidBody::get_static_friction() const {
    return m_component->static_friction;
}
void ScriptRigidBody::set_static_friction(double value) {
    m_component->static_friction = value;
    m_entity.update_rigid_body_static_friction(value);
}

double ScriptRigidBody::get_dynamic_friction() const {
    return m_component->dynamic_friction;
}
void ScriptRigidBody::set_dynamic_friction(double value) {
    m_component->dynamic_friction = value;
    m_entity.update_rigid_body_dynamic_friction(value);
}

double ScriptRigidBody::get_restitution() const {
    return m_component->restitution;
}
void ScriptRigidBody::set_restitution(double value) {
    m_component->restitution = value;
    m_entity.update_rigid_body_restitution(value);
}

void ScriptRigidBody::add_force(const glm::vec3& force) {
    auto& pm = Application::get().get_physics_manager();
    pm.add_force(m_entity, force);
}

} // namespace Helios::ScriptUserTypes
