#pragma once
#include "Helios/Scene/Entity.h"
namespace Helios {
class PhysicsBodyComponent;
}
namespace Helios::ScriptUserTypes {
class ScriptPhysicsBody {
  public:
    ScriptPhysicsBody(PhysicsBodyComponent* component, Entity entity)
        : m_component(component), m_entity(entity) {}

    double get_gravity_factor() const;
    void set_gravity_factor(double value);

    double get_friction() const;
    void set_friction(double value);

    double get_restitution() const;
    void set_restitution(double value);

    void add_force(const glm::vec3& force);

    PhysicsBodyComponent* get_component() { return m_component; }

  private:
    PhysicsBodyComponent* m_component;
    Entity m_entity;
};
} // namespace Helios::ScriptUserTypes
