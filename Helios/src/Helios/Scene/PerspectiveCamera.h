#pragma once
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/glm.hpp>

#include "Transform.h"

namespace Helios {
class PerspectiveCamera {
  public:
    glm::mat4 view_matrix;
    glm::mat4 view_no_translation_matrix;
    glm::mat4 projection_matrix;
    glm::mat4 view_projection_matrix;

    glm::vec3 position;

  public:
    PerspectiveCamera() = default;

    // Fov in degrees (not radians)
    PerspectiveCamera(const Transform& transform, float aspect_ratio,
                      float fovY, float _near, float _far);
};
} // namespace Helios
