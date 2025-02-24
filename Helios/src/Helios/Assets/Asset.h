#pragma once
#include <string>

namespace Helios {
class Asset {
  public:
    const std::string& get_name() const { return m_name; }

  protected:
    void init_asset(const std::string& name) { m_name = name; }

  protected:
    std::string m_name;
};
} // namespace Helios
