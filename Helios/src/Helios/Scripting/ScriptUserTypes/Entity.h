#pragma once
#include "Components.h"
#include "Helios/Scene/Entity.h"
namespace Helios::ScriptUserTypes {
class ScriptEntity {
  public:
    ScriptEntity(Entity entity) : m_entity(entity), m_components(entity) {}
    ScriptComponents* get_components() { return &m_components; }

  private:
    Entity m_entity;
    ScriptComponents m_components;
};
} // namespace Helios::ScriptUserTypes
