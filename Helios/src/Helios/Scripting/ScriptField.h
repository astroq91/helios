#include "Helios/Scripting/SerializableField.h"
#include "ScriptUserTypes/Entity.h"
#include <sol/sol.hpp>
namespace Helios {

enum class ScriptFieldType {
    Entity,
};

class ScriptField {
  public:
    virtual SerializableField* get_object() { return nullptr; }
    ScriptFieldType get_type() const { return m_type; }
    const std::string& get_name() const { return m_name; }
    bool updated() const { return m_updated; }

    template <typename Derived> Derived* as() {
        return static_cast<Derived*>(this);
    }

  protected:
    explicit ScriptField(const std::string& name, ScriptFieldType type)
        : m_name(name), m_type(type) {}

  protected:
    std::string m_name;
    ScriptFieldType m_type;
    bool m_updated = false;
};

class ScriptFieldEntity : public ScriptField {
  public:
    ScriptFieldEntity() = delete;
    ScriptFieldEntity(const std::string& name,
                      ScriptUserTypes::ScriptEntity* object)
        : ScriptField(name, ScriptFieldType::Entity), m_object(object) {}

    virtual ScriptUserTypes::ScriptEntity* get_object() override {
        return m_object;
    }

    void set_state(uint32_t entity) {
        m_entity = entity;
        m_updated = true;
    }
    uint32_t get_state() const { return m_entity; }

  private:
    uint32_t m_entity = k_no_entity;
    ScriptUserTypes::ScriptEntity* m_object;
};

} // namespace Helios
