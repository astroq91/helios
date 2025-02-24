#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace Math {
inline void decompose_transform(const glm::mat4& matrix, glm::vec3& position,
                                glm::quat& rotation, glm::vec3& scale) {
    // Extract position
    position = glm::vec3(matrix[3]);

    // Extract scale
    scale.x = glm::length(glm::vec3(matrix[0]));
    scale.y = glm::length(glm::vec3(matrix[1]));
    scale.z = glm::length(glm::vec3(matrix[2]));

    // Normalize the matrix to remove the scale
    glm::mat4 norm_matrix = matrix;
    norm_matrix[0] /= scale.x;
    norm_matrix[1] /= scale.y;
    norm_matrix[2] /= scale.z;

    // create a quaternion from the rotation matrix
    rotation = glm::quat_cast(norm_matrix);
}
} // namespace Math
