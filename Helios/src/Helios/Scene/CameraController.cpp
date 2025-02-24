#include "CameraController.h"

#include "Helios/Events/Input.h"

#include <GLFW/glfw3.h>

#include "Helios/Core/Application.h"

constexpr glm::vec3 k_world_up = glm::vec3(0.0f, 1.0f, 0.0f); // Fixed world up
constexpr float k_pitch_limit = 89.0f; // Clamp pitch to avoid flipping

namespace Helios {
glm::vec3 CameraController::get_forward() const {
    glm::vec3 front;
    front.x = -cos(glm::radians(m_transform->rotation.x)) *
              sin(glm::radians(m_transform->rotation.y));
    front.y = sin(glm::radians(m_transform->rotation.x));
    front.z = -cos(glm::radians(m_transform->rotation.x)) *
              cos(glm::radians(m_transform->rotation.y));
    front = glm::normalize(front);
    return front;
}

void CameraController::on_update(float ts) {
    if (Input::is_key_pressed(KeyCode::W)) {
        m_transform->position += m_movement_speed * ts * m_camera_front;
    }

    if (Input::is_key_pressed(KeyCode::S)) {
        m_transform->position -= m_movement_speed * ts * m_camera_front;
    }

    if (Input::is_key_pressed(KeyCode::A)) {
        m_transform->position -= m_movement_speed * ts * m_camera_right;
    }

    if (Input::is_key_pressed(KeyCode::D)) {
        m_transform->position += m_movement_speed * ts * m_camera_right;
    }

    if (Input::is_key_pressed(KeyCode::Space)) {
        m_transform->position += m_movement_speed * ts * m_camera_up;
    }

    if (Input::is_key_pressed(KeyCode::LeftShift)) {
        m_transform->position -= m_movement_speed * ts * m_camera_up;
    }

    if (m_rotation_locked) {
        return;
    }

    glm::vec2 mouse_pos = Input::get_mouse_pos();
    float x_offset = mouse_pos.x - m_last_mouse_pos.x;
    float y_offset =
        m_last_mouse_pos.y -
        mouse_pos.y; // reversed since y-coordinates range from bottom to top
    m_last_mouse_pos.x = mouse_pos.x;
    m_last_mouse_pos.y = mouse_pos.y;

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
        glm::normalize(m_transform->rotation * glm::vec3(0.0f, 0.0f, -1.0f));
    m_camera_right =
        glm::normalize(glm::cross(m_camera_front, glm::vec3(0.0f, 1.0f, 0.0f)));
    m_camera_up = glm::normalize(glm::cross(m_camera_right, m_camera_front));

    glm::quat pitchQuat =
        glm::angleAxis(glm::radians(y_offset), m_camera_right);
    glm::quat yawQuat =
        glm::angleAxis(glm::radians(-x_offset), glm::vec3(0.0f, 1.0f, 0.0f));
    m_transform->rotation =
        glm::normalize(yawQuat * pitchQuat * m_transform->rotation);
}

void CameraController::on_event(Event& e) {
    EventDispatcher dispatcher = EventDispatcher(e);
    dispatcher.dispatch<KeyPressedEvent>(
        HL_BIND_EVENT_FN(CameraController::on_key_pressed));
}

bool CameraController::on_key_pressed(KeyPressedEvent& e) {
    if (e.get_key_code() == KeyCode::D8) {
        set_locked_rotation(false);
        glfwSetInputMode(Application::get().get_native_window(), GLFW_CURSOR,
                         GLFW_CURSOR_DISABLED);
        m_last_mouse_pos = Input::get_mouse_pos();
    } else if (e.get_key_code() == KeyCode::D9) {
        set_locked_rotation(true);
        glfwSetInputMode(Application::get().get_native_window(), GLFW_CURSOR,
                         GLFW_CURSOR_NORMAL);
        m_last_mouse_pos = Input::get_mouse_pos();
    }

    return false;
}
} // namespace Helios
