#pragma once
#include <glm/glm.hpp>

#include <glm/gtx/quaternion.hpp>

namespace Helios {
struct Transform {
    glm::vec3 position = {0.0f, 0.0f, 0.0f};
    glm::quat rotation = {0.0f, 0.0f, 0.0f, 1.0f};
    glm::vec3 scale = {1.0f, 1.0f, 1.0f};

    glm::mat4 ToMat4() const {
        glm::mat4 rot = glm::toMat4(rotation);
        glm::mat4 model = glm::translate(glm::mat4(1.0f), position) * rot *
                          glm::scale(glm::mat4(1.0f), scale);

        return model;
    }

    glm::vec3 get_euler() const {
        return glm::degrees(glm::eulerAngles(rotation));
    }

    void set_euler(const glm::vec3& euler) {
        rotation = glm::quat(glm::radians(euler));
    }
};
} // namespace Helios
