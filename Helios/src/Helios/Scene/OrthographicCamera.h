#pragma once

#include "Helios/Scene/Transform.h"

#include <glm/glm.hpp>

class OrthographicCamera {
public:
    glm::mat4 view_matrix;
    glm::mat4 projection_matrix;
    glm::mat4 view_projection_matrix;

  public:
    OrthographicCamera() = default;

    // Fov in degrees (not radians)
    OrthographicCamera(const Helios::Transform& transform, float size,
                       float aspect_ratio, float z_near, float z_far);
};
