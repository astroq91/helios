#include "PerspectiveCamera.h"

namespace Helios {
PerspectiveCamera::PerspectiveCamera(const Transform& transform,
                                     float aspect_ratio, float fovY,
                                     float _near, float _far) {
    position = transform.position;
    view_matrix = glm::inverse(transform.ToMat4());
    view_no_translation_matrix = glm::mat4(glm::mat3(view_matrix));
    projection_matrix =
        glm::perspectiveRH_ZO(glm::radians(fovY), aspect_ratio, _near, _far);
    projection_matrix[1][1] *=
        -1; // Flip the Y axis, so it is easier to work with

    view_projection_matrix = projection_matrix * view_matrix;
}
} // namespace Helios
