#include "Helios/Scripting/SerializableField.h"
#include <cstdint>
#include <sol/sol.hpp>
#include <stduuid/uuid.h>
namespace Helios {

enum class ScriptFieldType {
    Entity,
};

typedef std::variant<uint32_t, double, bool, std::string> ScriptFieldValueType;

class ScriptField {
  public:
    explicit ScriptField(const std::string& name, ScriptFieldType type,
                         SerializableField* object)
        : m_name(name), m_type(type), m_object(object) {
        switch (type) {
        case ScriptFieldType::Entity: {
            m_value = UINT32_MAX;
            break;
        }
        }
    }
    template <typename T> T* get_object() { return static_cast<T*>(m_object); }
    ScriptFieldType get_type() const { return m_type; }
    const std::string& get_name() const { return m_name; }

    bool updated() const { return m_updated; }

    void set_value(const ScriptFieldValueType& value) {
        // TODO: Add type checks
        m_value = value;
        m_updated = true;
    }
    const ScriptFieldValueType& get_value() const { return m_value; }

  protected:
    std::string m_name;
    ScriptFieldValueType m_value;
    ScriptFieldType m_type;
    SerializableField* m_object;
    bool m_updated = false;
};

} // namespace Helios
