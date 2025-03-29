#pragma once
#include "Texture.h"
#include "Helios/Core/Core.h"
#include <glm/glm.hpp>
#include <filesystem>
#include <map>
#include <freetype/freetype.h>

namespace Helios {
    struct Character {
        SharedPtr<Texture> texture;
        glm::ivec2 size;
        glm::ivec2 bearing;
        uint32_t advance;
    };

    class Font {
      public:
        Font(const std::filesystem::path& path, FT_Library library);

        void set_pixel_size(uint32_t width, uint32_t height);
        void load_characters();

        const Character& get_charater(char c) const {
            return m_characters.at(c);
        };

      private:
        std::string m_name;
        FT_Face m_face;
        std::map<char, Character> m_characters;
    };
}