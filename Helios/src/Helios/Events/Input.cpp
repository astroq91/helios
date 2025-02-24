#include "Input.h"

#include "Helios/Core/Application.h"

namespace Helios {
bool Input::is_key_pressed(KeyCode key) {
  GLFWwindow *window = Application::get().get_native_window();
  const int state = glfwGetKey(window, (int)key);

  if (state == GLFW_PRESS) {
    return true;
  } else {
    return false;
  }
}

bool Input::is_key_released(KeyCode key) {
  GLFWwindow *window = Application::get().get_native_window();
  const int state = glfwGetKey(window, (int)key);

  if (state == GLFW_RELEASE) {
    return true;
  } else {
    return false;
  }
}

glm::vec2 Input::get_mouse_pos() {
  GLFWwindow *window = Application::get().get_native_window();

  double mouseX, mouseY;
  glfwGetCursorPos(window, &mouseX, &mouseY);

  return {mouseX, mouseY};
}
} // namespace Helios
