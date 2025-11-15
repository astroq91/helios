#pragma once
#include <glm/glm.hpp>

#include "Geometry.h"
#include "Helios/Scene/Transform.h"
#include "RigidBody.h"

#include "JoltImpls.h"

namespace Helios::Physics {

struct SceneInfo {
    float timestep = 1.0f / 60.0f;
    glm::vec3 gravity = {0.0f, -9.81f, 0.0f};
};

struct ActorInfo {
    RigidBodyType type;
    Geometry* geometry = nullptr;
    Transform transform;
    // physx::PxRigidDynamicLockFlags lock_flags;
    bool kinematic = false;

    float static_friction = 0.5f;
    float dynamic_friction = 0.5f;
    float restitution = 0.6f;
};

class PhysicsManager {
  public:
    ~PhysicsManager();
    void init();

    void set_scene(const SceneInfo& info = {});

    bool step(float ts);

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
};
} // namespace Helios::Physics
