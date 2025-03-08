#include "OrthographicCamera.h"

OrthographicCamera::OrthographicCamera(const Helios::Transform& transform,
                                       float size,
                                       float aspect_ratio, float z_near,
                                       float z_far) {
    view_matrix = glm::inverse(transform.ToMat4());
    projection_matrix =
        glm::orthoRH_ZO(-aspect_ratio * size, aspect_ratio * size, size, -size, z_near, z_far);
    projection_matrix[1][1] *=
        -1; // Flip the Y axis, so it is easier to work with

    view_projection_matrix = projection_matrix * view_matrix;
}