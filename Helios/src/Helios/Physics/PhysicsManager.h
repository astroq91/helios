#pragma once
#include <glm/glm.hpp>
#include <variant>

#include "Shapes.h"
#include "Helios/Scene/Transform.h"
#include "PhysicsBody.h"

#include "JoltImpls.h"

namespace Helios::Physics {

struct SceneInfo {
    float timestep = 1.0f / 60.0f;
    glm::vec3 gravity = {0.0f, -9.81f, 0.0f};
};

struct BodyInfo {
    PhysicsBodyType type;
    std::variant<BoxShape> shape;
    Transform transform;
    bool kinematic = false;

    float static_friction = 0.5f;
    float dynamic_friction = 0.5f;
    float restitution = 0.6f;
};

class PhysicsManager {
  public:
    ~PhysicsManager();
    void init() noexcept;

    void create_body(uint32_t entity, const BodyInfo& info) noexcept;
    void destroy_body(uint32_t entity) noexcept;
    void set_scene(const SceneInfo& info) noexcept;

    Transform get_transform(uint32_t entity) noexcept;
    void set_transform(uint32_t entity, const Transform& transform) noexcept;

    void set_gravity_factor(uint32_t entity, float value) noexcept;
    void set_friction(uint32_t entity, float value) noexcept;
    void set_restitution(uint32_t entity, float value) noexcept;
    bool step(float ts) noexcept;

    void add_force(uint32_t entity, const glm::vec3& force) {
        // TODO: implement
    }

  private:
    SceneInfo m_scene_info;

    JPH::PhysicsSystem m_physics_system;
    BPLayerInterfaceImpl m_broad_phase_layer_interface;
    ObjectVsBroadPhaseLayerFilterImpl m_object_vs_broadphase_layer_filter;
    ObjectLayerPairFilterImpl m_object_vs_object_layer_filter;
    MyContactListener m_contact_listener;
    MyBodyActivationListener m_body_activation_listener;

    std::unique_ptr<JPH::TempAllocatorImpl> m_temp_allocator = nullptr;
    std::unique_ptr<JPH::JobSystemThreadPool> m_job_system = nullptr;

    std::unordered_map<JPH::BodyID, uint32_t> m_body_forward_map;
    std::unordered_map<uint32_t, JPH::BodyID> m_body_backward_map;
};
} // namespace Helios::Physics
