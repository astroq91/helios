#pragma once
#include "Helios/Scene/Scene.h"

namespace Helios::ScriptUserTypes {
class ScriptUI {
  public:
    ScriptUI(Scene* scene) : m_scene(scene) {}

    void render_text(const std::string& text, const glm::vec2& position,
                     float scale, const glm::vec4& tint_color) {
        m_scene->render_text(text, position, scale, tint_color);
    }

    uint32_t get_window_width() const {
        return m_scene->get_game_viewport_width();
    }

    uint32_t get_window_height() const {
        return m_scene->get_game_viewport_height();
    }

  private:
    Scene* m_scene;
};

} // namespace Helios::ScriptUserTypes
