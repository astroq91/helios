#pragma once

#include <lua.hpp>
#include <sol/sol.hpp>

#include "Helios/Assets/Asset.h"
#include "Helios/Scene/Entity.h"

enum class ScriptType { Entity, Global };
enum class ScriptLoadType { Source, File };

namespace Helios {
class Script : public Asset {
  public:
    Script(const std::string& src, Entity entity, ScriptLoadType load_type,
           ScriptType type = ScriptType::Entity);

    void on_start();
    void on_update(float ts);

  private:
    void load_script(const std::string& src, ScriptLoadType load_type);
    void expose_basic_types();
    void expose_functions();
    void expose_helios_user_types();
    void expose_component_user_types();
    void expose_key_codes();
    void set_globals();

  private:
    sol::state m_state;
    Entity m_entity;
};
} // namespace Helios
