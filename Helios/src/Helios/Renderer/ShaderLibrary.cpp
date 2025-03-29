#include "ShaderLibrary.h"

namespace Helios {
void ShaderLibrary::add_shader(const SharedPtr<Shader> &shader) {
  if (!m_shaders.contains(shader->get_name())) {
    m_shaders[shader->get_name()] = shader;
  } else {
    HL_ERROR(
        "Tried to add shader with name: {0}, but that shader already exists.",
        shader->get_name());
  }
}

SharedPtr<Shader> ShaderLibrary::get_shader(const std::string &name) {
  const auto it = m_shaders.find(name);

  if (it == m_shaders.end()) {
    HL_ERROR("No shader with name: '{0}'", name);
    return nullptr;
  } else {
    return it->second;
  }
}
} // namespace Helios
