#include "ScriptUserTypes/Entity.h"
#include <sol/sol.hpp>
namespace Helios {

enum class ScriptFieldType {
    Entity,
};

class ScriptField {
  public:
    explicit ScriptField(ScriptFieldType type, sol::object object)
        : m_type(type), m_object(object) {}
    sol::object get_object() { return m_object; }
    ScriptFieldType get_type() const { return m_type; }
    template <typename T> T& as() { return dynamic_cast<T&>(*this); }

  private:
    ScriptFieldType m_type;
    sol::object m_object;
};

class EntityScriptField : public ScriptField {
  public:
    EntityScriptField(sol::object object)
        : ScriptField(ScriptFieldType::Entity, object) {}

    void set_field(uint32_t entity) { m_entity = entity; }
    uint32_t get_field() const { return m_entity; }

  private:
    uint32_t m_entity;
};

} // namespace Helios
