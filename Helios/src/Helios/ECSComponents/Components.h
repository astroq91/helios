#pragma once

#include <glm/glm.hpp>
#include <stduuid/uuid.h>

#include "Helios/Physics/RigidBody.h"
#include "Helios/Renderer/Light.h"
#include "Helios/Renderer/Material.h"
#include "Helios/Renderer/Mesh.h"
#include "Helios/Scene/Transform.h"
#include "Helios/Scripting/Script.h"

namespace Helios {

enum class MeshType { Cube };

struct PersistentIdComponent {
    PersistentIdComponent() : m_id(uuids::uuid_system_generator{}()) {}
    PersistentIdComponent(const uuids::uuid& id) : m_id(id) {}

    const uuids::uuid& get_id() const { return m_id; }

  private:
    uuids::uuid m_id;
};

struct TransformComponent {
    glm::vec3 position = {0.0f, 0.0f, 0.0f};
    glm::quat rotation = {0.0f, 0.0f, 0.0f, 1.0f};
    glm::vec3 scale = {1.0f, 1.0f, 1.0f};

    glm::vec3 local_position = {0.0f, 0.0f, 0.0f};
    glm::quat local_rotation = {0.0f, 0.0f, 0.0f, 1.0f};
    glm::vec3 local_scale = {1.0f, 1.0f, 1.0f};

    TransformComponent() = default;

    TransformComponent(const Transform& transform) {
        position = transform.position;
        rotation = transform.rotation;
        scale = transform.scale;
    }

    Transform to_transform() const {
        return Transform{position, rotation, scale};
    }

    Transform to_transform_local() const {
        return Transform{local_position, local_rotation, local_scale};
    }

    glm::vec3 get_euler() const {
        return glm::degrees(glm::eulerAngles(rotation));
    }

    glm::vec3 get_euler_local() const {
        return glm::degrees(glm::eulerAngles(local_rotation));
    }

    void set_euler(const glm::vec3& euler) {
        rotation = glm::quat(glm::radians(euler));
    }
    void set_euler_local(const glm::vec3& euler) {
        local_rotation = glm::quat(glm::radians(euler));
    }
};

struct NameComponent {
    std::string name;
};

struct CameraComponent {
    float fovY = 120.0f;
    float z_near = 0.1f;
    float z_far = 100.0f;
};

struct MeshRendererComponent {
    SharedPtr<Mesh> mesh = nullptr;
    SharedPtr<Material> material = nullptr;
    glm::vec4 tint_color = glm::vec4(1);
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

struct ExposedFieldEntry {
    std::string name;
    ScriptFieldType type;
    std::variant<uuids::uuid, double, bool, std::string> value;
};

struct ScriptComponent {
    std::unique_ptr<Script> script = nullptr;
    std::vector<ExposedFieldEntry> exposed_fields;
};

struct RigidBodyComponent {
    RigidBodyType type = RigidBodyType::Dynamic;
    float mass = 1.0f;
    bool kinematic = false;

    float static_friction = 0.5f;
    float dynamic_friction = 0.5f;
    float restitution = 0.6f;

    bool override_dynamic_physics = false;
    bool lock_linear_x = false;
    bool lock_linear_y = false;
    bool lock_linear_z = false;
    bool lock_angular_x = false;
    bool lock_angular_y = false;
    bool lock_angular_z = false;
};

struct BoxColliderComponent {
    glm::vec3 size = {0.5f, 0.5, 0.5f};
};

struct ParentComponent {
    explicit ParentComponent(uint32_t _parent) : parent(_parent) {}
    const uint32_t parent;
};

} // namespace Helios
