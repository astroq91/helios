#pragma once
#include <string>

#include "Shader.h"

namespace Helios {
class ShaderLibrary {
public:
  void add_shader(const Ref<Shader> &shader);

  Ref<Shader> get_shader(const std::string &name);

private:
  std::map<std::string, Ref<Shader>> m_shaders;
};
} // namespace Helios
