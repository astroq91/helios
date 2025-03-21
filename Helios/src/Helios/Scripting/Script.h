#pragma once

#include <lua.hpp>
#include <sol/sol.hpp>

#include "Helios/Assets/Asset.h"
#include "Helios/Scene/Entity.h"
#include "Helios/Scripting/ScriptField.h"

enum class ScriptType { Entity, Global };
enum class ScriptLoadType { Source, File };

namespace Helios {
class Script : public Asset {
  public:
    Script(const std::string& src, ScriptLoadType load_type, Scene* scene,
           Entity entity);

    void on_start();
    void on_update(float ts);

    std::vector<ScriptFieldEntity>& get_exposed_fields_entity() {
        return m_exposed_fields_entity;
    }

  private:
    void load_script(const std::string& src, ScriptLoadType load_type);
    void expose_basic_types();
    void expose_functions();
    void expose_helios_user_types();
    void expose_component_user_types();
    void expose_key_codes();
    void set_globals();
    void load_globals();

    void get_exposed_fields_state();
    void set_exposed_fields_state();

  private:
    sol::state m_state;
    Scene* m_scene;
    Entity m_entity;
    std::vector<ScriptFieldEntity> m_exposed_fields_entity;
};
} // namespace Helios
