#pragma once
#include "Camera.h"
#include "Helios/Events/Input.h"
#include "Helios/Events/KeyEvents.h"
#include "Helios/Events/MouseEvents.h"
#include "Transform.h"

namespace Helios {

struct SceneCameraInfo {
    uint32_t viewport_width;
    uint32_t viewport_height;
    Transform initial_transform = {};
    float movement_speed = 1.0f;
    float sensitivity = 1.0f;
    float fov_y = 120.0f;
    float near = 0.1f;
    float far = 100.0f;
};

class SceneCamera {
  public:
    SceneCamera(const SceneCameraInfo& info)
        : m_initial_transform(info.initial_transform),
          m_movement_speed(info.movement_speed),
          m_sensitivity(info.sensitivity), m_fov_y(info.fov_y),
          m_near(info.near), m_far(info.far),
          m_aspect_ratio(static_cast<float>(info.viewport_width) /
                         static_cast<float>(info.viewport_height)) {
        m_last_mouse_pos = Input::get_mouse_pos();
    }

    /**
     * \brief Lock, or unlock, the camera rotation.
     * \param locked Bool value to control locked state.
     */
    void set_locked_rotation(bool locked) { m_rotation_locked = locked; }

    float* get_movement_speed() { return &m_movement_speed; }
    float* get_sensitivity() { return &m_sensitivity; }

    Transform get_transform() const { return m_transform; }
    void reset_camera();

    glm::vec3 get_forward() const;

    float get_fov_y() const { return m_fov_y; }
    float get_near() const { return m_near; }
    float get_far() const { return m_far; }

    Camera get_camera() const {
        return Camera(m_transform, m_aspect_ratio, m_fov_y, m_near, m_far);
    }

    /**
     * \brief Call every frame to update camera.
     * \param ts The timestep.
     */
    void on_update(float ts);

    /**
     * \brief Call every time the viewport changes in size.
     * \param viewport_width The new width.
     * \param viewport_height The new height.
     */
    void on_resize(uint32_t viewport_width, uint32_t viewport_height);

    /**
     * \brief Call in on_event to handle events.
     * \param e The event.
     */
    void on_event(Event& e);

  private:
    void update_camera_vectors();
    bool on_key_pressed(KeyPressedEvent& e);

    bool on_mouse_pressed(MouseButtonPressedEvent& e);
    bool on_mouse_released(MouseButtonReleasedEvent& e);

  private:
    float m_pitch = 0.0f;
    float m_yaw = 0.0f;

    glm::vec3 m_camera_front = glm::vec3(0, 0, -1);
    glm::vec3 m_camera_up = glm::vec3(0, 1, 0);
    glm::vec3 m_camera_right = glm::vec3(1, 0, 0);

    const Transform m_initial_transform;
    Transform m_transform;
    float m_movement_speed;
    float m_sensitivity;

    float m_fov_y;
    float m_near;
    float m_far;

    float m_aspect_ratio;

    bool m_rotation_locked = true;

    glm::vec2 m_last_mouse_pos;
};
} // namespace Helios
