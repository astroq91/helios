#include "Font.h"
#include "FontLibrary.h"
#include "Helios/Core/Log.h"

namespace Helios {

Font::Font(const std::filesystem::path& path, FT_Library library) {
    if (!library) {
        return;
    }
    m_name = path.string();

    if (FT_New_Face(library, path.string().c_str(), 0, &m_face)) {
        HL_ERROR("Failed to create font face from {}.", path.string());
    }
}

void Font::set_pixel_size(uint32_t width, uint32_t height) {
    if (FT_Set_Pixel_Sizes(m_face, width, height)) {
        HL_ERROR("Failed to set pixel size for font");
    }
}

void Font::load_characters() {
    for (unsigned char c = 0; c < 128; c++) {
        if (FT_Load_Char(m_face, c, FT_LOAD_RENDER)) {
            HL_ERROR("Failed to load glyph '{}'", c);
            continue;
        }

        SharedPtr<Texture> texture = nullptr;

        if (m_face->glyph->bitmap.width > 0 && m_face->glyph->bitmap.rows > 0) {
            std::stringstream stream;
            stream << m_name << "_" << c;

            texture = Texture::create(
                stream.str(), m_face->glyph->bitmap.buffer,
                m_face->glyph->bitmap.width, m_face->glyph->bitmap.rows,
                m_face->glyph->bitmap.width * m_face->glyph->bitmap.rows,
                VK_FORMAT_R8_UNORM);
        }

        m_characters.insert(std::pair<char, Character>(
            c, Character{
                   .texture = texture,
                   .size = {m_face->glyph->bitmap.width,
                            m_face->glyph->bitmap.rows},
                   .bearing = {m_face->glyph->bitmap_left,
                               m_face->glyph->bitmap_top},
                   .advance = static_cast<uint32_t>(m_face->glyph->advance.x),
               }));
    }
}
} // namespace Helios
