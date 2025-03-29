#include "TextureLibrary.h"
#include "Texture.h"

namespace Helios {
void TextureLibrary::add_texture(const SharedPtr<Texture> &texture) {
  if (!m_textures.contains(texture->get_name())) {
      m_textures[texture->get_name()] = texture;
  } else {
    HL_ERROR(
        "Tried to add texture with name: {0}, but that texture already exists.",
        texture->get_name());
  }
}

SharedPtr<Texture> TextureLibrary::get_texture(const std::string &name) {
  const auto it = m_textures.find(name);

  if (it == m_textures.end()) {
    HL_ERROR("No texture with name: '{0}'", name);
    return nullptr;
  } else {
    return it->second;
  }
}
} // namespace Helios
