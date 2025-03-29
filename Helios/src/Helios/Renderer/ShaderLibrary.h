#pragma once
#include <string>

#include "Shader.h"

namespace Helios {
class ShaderLibrary {
public:
  void add_shader(const SharedPtr<Shader> &shader);

  SharedPtr<Shader> get_shader(const std::string &name);

private:
  std::map<std::string, SharedPtr<Shader>> m_shaders;
};
} // namespace Helios
