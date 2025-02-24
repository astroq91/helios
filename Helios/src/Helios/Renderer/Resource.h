#pragma once

#define UUID_SYSTEM_GENERATOR
#include <stduuid/uuid.h>

namespace Helios {
class Resource {
public:
  uuids::uuid get_uuid() const { return m_uuid; }

protected:
  void init_uuid() { m_uuid = uuids::uuid_system_generator{}(); }

private:
  uuids::uuid m_uuid;
};
} // namespace Helios
