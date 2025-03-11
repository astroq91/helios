#include "FontLibrary.h"

namespace Helios {
void FontLibrary::init() {
    if (FT_Init_FreeType(&m_library)) {
        HL_ERROR("Failed to init freetype");
    }
}

Ref<Font> FontLibrary::load_font(const std::filesystem::path& path) {
    m_fonts.insert(std::pair<std::string, Ref<Font>>(
        path.string(), make_ref<Font>(path, m_library)));
    return m_fonts.at(path.string());
}
} // namespace Helios
