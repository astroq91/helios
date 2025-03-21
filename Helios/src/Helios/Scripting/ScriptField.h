#include <sol/sol.hpp>
namespace Helios {
namespace ScriptUserTypes {
class ScriptEntity;
}

class ScriptField {
  public:
    explicit ScriptField(const std::string& name) : m_name(name) {}
    const std::string& get_name() const { return m_name; }

  private:
    std::string m_name;
}; // namespace ScriptUserTypes

class ScriptFieldEntity : public ScriptField {
  public:
    ScriptFieldEntity(const std::string& name,
                      ScriptUserTypes::ScriptEntity* object)
        : ScriptField(name), m_object(object) {}

    void set_field(uint32_t entity) { m_entity = entity; }
    uint32_t get_field() const { return m_entity; }
    ScriptUserTypes::ScriptEntity* get_object() { return m_object; }

  private:
  public:
    ScriptUserTypes::ScriptEntity* m_object;
    uint32_t m_entity;
};

} // namespace Helios
