#pragma once
#include "Components.h"
#include "Helios/Scene/Entity.h"
#include "Helios/Scripting/SerializableField.h"
namespace Helios::ScriptUserTypes {
class ScriptEntity : public SerializableField {
  public:
    ScriptEntity(Entity entity) : m_entity(entity), m_components(entity) {}
    ScriptComponents* get_components() { return &m_components; }
    uint32_t get_id() const { return m_entity; }

    void set_entity(Entity entity) { m_entity = entity; }

  private:
    Entity m_entity;
    ScriptComponents m_components;
};
} // namespace Helios::ScriptUserTypes
