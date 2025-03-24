#include "Input.h"
#include "Helios/Events/Input.h"

namespace Helios::ScriptUserTypes {
bool ScriptInput::is_key_pressed(KeyCode key) {
    if (!m_scene || !m_scene->is_game_viewport_focused()) {
        return false;
    }
    return Input::is_key_pressed(key);
}
bool ScriptInput::is_key_released(KeyCode key) {
    if (!m_scene || !m_scene->is_game_viewport_focused()) {
        return false;
    }
    return Input::is_key_released(key);
}

glm::vec2 ScriptInput::get_mouse_pos() {
    if (!m_scene || !m_scene->is_game_viewport_focused()) {
        return {0, 0};
    }
    return Input::get_mouse_pos();
}
void ScriptInput::set_mouse_pos(double x_pos, double y_pos) {
    if (!m_scene || !m_scene->is_game_viewport_focused()) {
        return;
    }
    Input::set_mouse_pos(x_pos, y_pos);
}

} // namespace Helios::ScriptUserTypes
