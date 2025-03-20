#include "SceneCamera.h"

#include "GLFW/glfw3.h"
#include "Helios/Core/Application.h"
#include "Helios/Events/MouseEvents.h"
#include "glm/ext/quaternion_geometric.hpp"
#include "glm/geometric.hpp"

constexpr glm::vec3 k_world_up = glm::vec3(0.0f, 1.0f, 0.0f); // Fixed world up
constexpr float k_pitch_limit = 89.0f; // Clamp pitch to avoid flipping

constexpr float k_extra_speed = 2;

namespace Helios {
glm::vec3 SceneCamera::get_forward() const {
    glm::vec3 front;
    front.x = -cos(glm::radians(m_transform.rotation.x)) *
              sin(glm::radians(m_transform.rotation.y));
    front.y = sin(glm::radians(m_transform.rotation.x));
    front.z = -cos(glm::radians(m_transform.rotation.x)) *
              cos(glm::radians(m_transform.rotation.y));
    front = glm::normalize(front);
    return front;
}

void SceneCamera::on_update(float ts) {
    if (Input::is_key_pressed(KeyCode::W)) {
        m_transform.position += m_movement_speed * ts * m_camera_front;
    }

    if (Input::is_key_pressed(KeyCode::S)) {
        m_transform.position -= m_movement_speed * ts * m_camera_front;
    }

    if (Input::is_key_pressed(KeyCode::A)) {
        m_transform.position -= m_movement_speed * ts * m_camera_right;
    }

    if (Input::is_key_pressed(KeyCode::D)) {
        m_transform.position += m_movement_speed * ts * m_camera_right;
    }

    if (Input::is_key_pressed(KeyCode::Enter)) {
        m_transform.position += m_movement_speed * ts * m_camera_up;
    }

    if (Input::is_key_pressed(KeyCode::LeftShift)) {
        m_transform.position -= m_movement_speed * ts * m_camera_up;
    }

    if (m_rotation_locked) {
        return;
    }

    glm::vec2 mouse_pos = Input::get_mouse_pos();
    float x_offset = mouse_pos.x - m_last_mouse_pos.x;
    float y_offset =
        m_last_mouse_pos.y -
        mouse_pos.y; // reversed since y-coordinates range from bottom to top

    x_offset *= m_sensitivity * ts;
    y_offset *= m_sensitivity * ts;

    m_yaw += x_offset;
    m_pitch += y_offset;

    if (m_pitch > k_pitch_limit) {
        m_pitch = k_pitch_limit;
        y_offset = 0;
    } else if (m_pitch < -k_pitch_limit) {
        m_pitch = -k_pitch_limit;
        y_offset = 0;
    }

    m_camera_front =
        glm::normalize(m_transform.rotation * glm::vec3(0.0f, 0.0f, -1.0f));
    m_camera_right =
        glm::normalize(glm::cross(m_camera_front, glm::vec3(0.0f, 1.0f, 0.0f)));
    m_camera_up = glm::normalize(glm::cross(m_camera_right, m_camera_front));

    glm::quat pitchQuat =
        glm::angleAxis(glm::radians(y_offset), m_camera_right);
    glm::quat yawQuat =
        glm::angleAxis(glm::radians(-x_offset), glm::vec3(0.0f, 1.0f, 0.0f));
    m_transform.rotation =
        glm::normalize(yawQuat * pitchQuat * m_transform.rotation);

    Input::set_mouse_pos(m_last_mouse_pos.x, m_last_mouse_pos.y);
}

void SceneCamera::reset_camera() {
    m_transform = m_initial_transform;
    m_pitch = 0;
    m_yaw = 0;
}

void SceneCamera::on_resize(uint32_t viewport_width, uint32_t viewport_height) {
    m_aspect_ratio = static_cast<float>(viewport_width) /
                     static_cast<float>(viewport_height);
}

void SceneCamera::on_event(Event& e) {
    EventDispatcher dispatcher = EventDispatcher(e);
    dispatcher.dispatch<KeyPressedEvent>(
        HL_BIND_EVENT_FN(SceneCamera::on_key_pressed));
    dispatcher.dispatch<KeyReleasedEvent>(
        HL_BIND_EVENT_FN(SceneCamera::on_key_released));
    dispatcher.dispatch<MouseButtonPressedEvent>(
        HL_BIND_EVENT_FN(SceneCamera::on_mouse_pressed));
    dispatcher.dispatch<MouseButtonReleasedEvent>(
        HL_BIND_EVENT_FN(SceneCamera::on_mouse_released));
}

bool SceneCamera::on_key_pressed(KeyPressedEvent& e) {
    if (e.get_key_code() == KeyCode::LeftControl) {
        m_movement_speed *= k_extra_speed;
    }

    return false;
}

bool SceneCamera::on_key_released(KeyReleasedEvent& e) {
    if (e.get_key_code() == KeyCode::LeftControl) {
        m_movement_speed /= k_extra_speed;
    }

    return false;
}

bool SceneCamera::on_mouse_pressed(MouseButtonPressedEvent& e) {
    if (e.get_button() == MouseButton::Right) {
        set_locked_rotation(false);
        glfwSetInputMode(Application::get().get_native_window(), GLFW_CURSOR,
                         GLFW_CURSOR_DISABLED);
        m_last_mouse_pos = Input::get_mouse_pos();
    }
    return false;
}

bool SceneCamera::on_mouse_released(MouseButtonReleasedEvent& e) {
    if (e.get_button() == MouseButton::Right) {
        set_locked_rotation(true);
        glfwSetInputMode(Application::get().get_native_window(), GLFW_CURSOR,
                         GLFW_CURSOR_NORMAL);
        m_last_mouse_pos = Input::get_mouse_pos();
    }
    return false;
}
} // namespace Helios
