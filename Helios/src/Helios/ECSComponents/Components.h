#pragma once

#include <glm/glm.hpp>

#include "Helios/Physics/PhysicsManager.h"
#include "Helios/Physics/RigidBody.h"
#include "Helios/Renderer/Light.h"
#include "Helios/Renderer/Material.h"
#include "Helios/Renderer/Mesh.h"
#include "Helios/Scene/Transform.h"
#include "Helios/Scripting/Script.h"

namespace Helios {

enum class MeshType { Cube };

struct TransformComponent {
    glm::vec3 position = {0.0f, 0.0f, 0.0f};
    glm::quat rotation = {0.0f, 0.0f, 0.0f, 1.0f};
    glm::vec3 scale = {1.0f, 1.0f, 1.0f};

    TransformComponent() = default;

    TransformComponent(const Transform& transform) {
        position = transform.position;
        rotation = transform.rotation;
        scale = transform.scale;
    }

    Transform to_transform() const {
        return Transform{position, rotation, scale};
    }

    glm::vec3 get_euler() const {
        return glm::degrees(glm::eulerAngles(rotation));
    }

    void set_euler(const glm::vec3& euler) {
        rotation = glm::quat(glm::radians(euler));
    }
};

struct NameComponent {
    std::string name;
};

struct CameraComponent {
    float fovY = 120.0f;
    float near = 0.1f;
    float far = 100.0f;
};

struct MeshComponent {
    Ref<Mesh> mesh = nullptr;
    Material material;
};

struct DirectionalLightComponent {
    glm::vec3 direction = glm::vec3(1.0f); // From the light source
    glm::vec3 ambient = glm::vec3(1.0f);
    glm::vec3 diffuse = glm::vec3(1.0f);
    glm::vec3 specular = glm::vec3(1.0f);

    DirectionalLight to_directional_light() const {
        return DirectionalLight{
            .direction = direction,
            .ambient = ambient,
            .diffuse = diffuse,
            .specular = specular,
        };
    }
};

struct PointLightComponent {
    glm::vec3 position = glm::vec3(0.0f);

    // Co-efficients for the fading of the point light, with respect to
    // distance.
    float constant = 1.0f;
    float linear = 0.09f;
    ;
    float quadratic = 0.032f;

    glm::vec3 ambient = glm::vec3(1.0f);
    glm::vec3 diffuse = glm::vec3(1.0f);
    glm::vec3 specular = glm::vec3(1.0f);

    PointLight to_point_light() const {
        return PointLight{
            .position = position,
            .constant = constant,
            .linear = linear,
            .quadratic = quadratic,
            .ambient = ambient,
            .diffuse = diffuse,
            .specular = specular,
        };
    }
};

struct ScriptComponent {
    Unique<Script> script = nullptr;
};

struct RigidBodyComponent {
    RigidBodyType type = RigidBodyType::Dynamic;
    float mass = 1.0f;
    bool kinematic = false;

    float static_friction = 0.5f;
    float dynamic_friction = 0.5f;
    float restitution = 0.6f;

    bool override_dynamic_physics = false;
};

struct BoxColliderComponent {
    glm::vec3 size = {0.5f, 0.5, 0.5f};
};

} // namespace Helios
