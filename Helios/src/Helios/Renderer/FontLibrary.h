#pragma once

#include "Font.h"
#include <freetype/freetype.h>
#include <filesystem>
#include <map>

namespace Helios {

class FontLibrary {
  public:
    void init();

    SharedPtr<Font> load_font(const std::filesystem::path& path);

  private:
    FT_Library m_library = nullptr;
    std::map<std::string, SharedPtr<Font>> m_fonts;
};
} // namespace Helios
