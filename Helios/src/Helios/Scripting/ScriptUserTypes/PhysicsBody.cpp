#include "PhysicsBody.h"
#include "Helios/Core/Application.h"
#include "Helios/ECSComponents/Components.h"

namespace Helios::ScriptUserTypes {

double ScriptPhysicsBody::get_gravity_factor() const {
    return m_component->gravity_factor;
}
void ScriptPhysicsBody::set_gravity_factor(double value) {
    m_component->gravity_factor = value;
    m_entity.update_physics_body_gravity_factor(value);
}

double ScriptPhysicsBody::get_friction() const {
    return m_component->static_friction;
}

void ScriptPhysicsBody::set_friction(double value) {
    m_component->dynamic_friction = value;
    m_entity.update_physics_body_friction(value);
}

double ScriptPhysicsBody::get_restitution() const {
    return m_component->restitution;
}
void ScriptPhysicsBody::set_restitution(double value) {
    m_component->restitution = value;
    m_entity.update_physics_body_restitution(value);
}

void ScriptPhysicsBody::add_force(const glm::vec3& force) {
    auto& pm = Application::get().get_physics_manager();
    pm.add_force(m_entity, force);
}

} // namespace Helios::ScriptUserTypes
