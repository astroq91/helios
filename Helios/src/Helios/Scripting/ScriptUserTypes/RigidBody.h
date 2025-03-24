#pragma once
#include "Helios/Scene/Entity.h"
namespace Helios {
class RigidBodyComponent;
}
namespace Helios::ScriptUserTypes {
class ScriptRigidBody {
  public:
    ScriptRigidBody(RigidBodyComponent* component, Entity entity)
        : m_component(component), m_entity(entity) {}

    double get_mass() const;
    void set_mass(double value);

    double get_static_friction() const;
    void set_static_friction(double value);

    double get_dynamic_friction() const;
    void set_dynamic_friction(double value);

    double get_restitution() const;
    void set_restitution(double value);

    void add_force(const glm::vec3& force);

    RigidBodyComponent* get_component() { return m_component; }

  private:
    RigidBodyComponent* m_component;
    Entity m_entity;
};
} // namespace Helios::ScriptUserTypes
