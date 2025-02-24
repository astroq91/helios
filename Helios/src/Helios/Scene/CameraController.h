#pragma once
#include "Helios/ECSComponents/Components.h"
#include "Helios/Events/Event.h"
#include "Helios/Events/Input.h"
#include "Helios/Events/KeyEvents.h"
#include "Transform.h"

namespace Helios {
class CameraController {
  public:
    CameraController()
        : m_transform(nullptr), m_movement_speed(1.0f), m_sensitivity(1.0f) {
        m_last_mouse_pos = Input::get_mouse_pos();
    }

    CameraController(TransformComponent* transform, float movement_speed,
                     float sensitivity)
        : m_transform(transform), m_movement_speed(movement_speed),
          m_sensitivity(sensitivity) {
        m_last_mouse_pos = Input::get_mouse_pos();
    }

    /**
     * \brief Lock, or unlock, the camera rotation.
     * \param locked Bool value to control locked state.
     */
    void set_locked_rotation(bool locked) { m_rotation_locked = locked; }

    float* get_movement_speed() { return &m_movement_speed; }
    float* get_sensitivity() { return &m_sensitivity; }

    Transform get_transform() const { return m_transform->to_transform(); }
    glm::vec3 get_forward() const;

    /**
     * \brief Call every frame to update camera.
     * \param ts The timestep.
     */
    void on_update(float ts);

    /**
     * \brief Call in on_event to handle events.
     * \param e The event.
     */
    void on_event(Event& e);

  private:
    void update_camera_vectors();
    bool on_key_pressed(KeyPressedEvent& e);

  private:
    float m_pitch = 0.0f;
    float m_yaw = 0.0f;

    glm::vec3 m_camera_front = glm::vec3(0, 0, -1);
    glm::vec3 m_camera_up = glm::vec3(0, 1, 0);
    glm::vec3 m_camera_right = glm::vec3(1, 0, 0);

    TransformComponent* m_transform;
    float m_movement_speed;
    float m_sensitivity;

    bool m_rotation_locked = true;

    glm::vec2 m_last_mouse_pos;
};
} // namespace Helios
