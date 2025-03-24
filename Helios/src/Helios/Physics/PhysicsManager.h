#pragma once
#include <PxPhysicsAPI.h>
#include <glm/glm.hpp>
#include <map>

#include "Geometry.h"
#include "Helios/Scene/Transform.h"
#include "RigidBody.h"

namespace Helios::Physics {

struct SceneInfo {
    float timestep = 1.0f / 60.0f;
    glm::vec3 gravity = {0.0f, -9.81f, 0.0f};
};

struct ActorInfo {
    RigidBodyType type;
    Geometry* geometry = nullptr;
    Transform transform;
    physx::PxRigidDynamicLockFlags lock_flags;
    bool kinematic = false;

    float static_friction = 0.5f;
    float dynamic_friction = 0.5f;
    float restitution = 0.6f;
};

class PhysicsManager {
  public:
    ~PhysicsManager();
    void init();

    void new_scene(const SceneInfo& info = {});
    void add_actor(uint32_t entity, const ActorInfo& info);
    void remove_actor(uint32_t entity);

    bool step(float ts);
    void set_actor_transform(uint32_t entity, const Transform& transform);
    Transform get_actor_transform(uint32_t entity);

    void set_rigid_dynamic_mass(uint32_t entity, float value);
    void set_material_static_friction(uint32_t entity, float value) {
        m_materials[entity]->setStaticFriction(value);
    }
    void set_material_dynamic_friction(uint32_t entity, float value) {
        m_materials[entity]->setDynamicFriction(value);
    }
    void set_material_restitution(uint32_t entity, float value) {
        m_materials[entity]->setRestitution(value);
    }
    void add_force(uint32_t entity, const glm::vec3& force);

  private:
    physx::PxPhysics* m_physics = nullptr;
    physx::PxFoundation* m_foundation = nullptr;
    physx::PxScene* m_scene = nullptr;
    physx::PxPvd* m_pvd = nullptr;

    float m_time_acc = 0.0f;
    SceneInfo m_scene_info;

    std::map<uint32_t, physx::PxActor*> m_actors;
    std::map<uint32_t, physx::PxMaterial*> m_materials;
    std::map<uint32_t, physx::PxShape*> m_shapes;
};
} // namespace Helios::Physics
