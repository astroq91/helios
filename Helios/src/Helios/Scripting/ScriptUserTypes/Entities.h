#pragma once
#include "Entity.h"
#include "Helios/Scene/Scene.h"
#include <sol/sol.hpp>

namespace Helios::ScriptUserTypes {
class ScriptEntities {
  public:
    ScriptEntities(Scene* scene, sol::state* state)
        : m_scene(scene), m_state(state) {}
    ScriptEntity create_entity(const std::string& name) {
        Entity entity = m_scene->create_entity(name);
        return ScriptEntity(entity);
    }

  private:
    Scene* m_scene;
    sol::state* m_state;
};

} // namespace Helios::ScriptUserTypes
