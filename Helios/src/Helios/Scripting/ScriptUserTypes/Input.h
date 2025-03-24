#pragma once
#include "Helios/Scene/Scene.h"
namespace Helios::ScriptUserTypes {
class ScriptInput {
  public:
    ScriptInput(Scene* scene) : m_scene(scene) {}
    bool is_key_pressed(KeyCode key);
    bool is_key_released(KeyCode key);

    glm::vec2 get_mouse_pos();
    void set_mouse_pos(double x_pos, double y_pos);

  private:
    Scene* m_scene;
};
} // namespace Helios::ScriptUserTypes
