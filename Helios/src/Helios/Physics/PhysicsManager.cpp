#include "PhysicsManager.h"
#include "Helios/Core/Log.h"
#include "Helios/Physics/RigidBody.h"
#include "PxMaterial.h"
#include "PxRigidActor.h"
#include "PxRigidBody.h"
#include "PxRigidDynamic.h"
#include "PxRigidStatic.h"
#include "PxSceneDesc.h"
#include "PxShape.h"
#include "common/PxTypeInfo.h"
#include "extensions/PxDefaultCpuDispatcher.h"
#include "extensions/PxExtensionsAPI.h"
#include "extensions/PxSimpleFactory.h"
#include "pvd/PxPvdSceneClient.h"
#include "pvd/PxPvdTransport.h"
#include <PxPhysicsAPI.h>

using namespace physx;

PxDefaultErrorCallback g_default_error_callback;
PxDefaultAllocator g_default_allocator_callback;
PxDefaultCpuDispatcher* g_dispatcher = nullptr;

constexpr PxSimulationFilterShader m_default_filter_shader =
    physx::PxDefaultSimulationFilterShader;
constexpr PxReal k_timestep = 1.0f / 60.0f;
constexpr const char* k_pvd_host = "127.0.0.1";

namespace Helios::Physics {
PhysicsManager::~PhysicsManager() {
    if (m_scene) {
        m_scene->release();
        g_dispatcher->release();
    }
    m_physics->release();
    if (m_pvd) {
        PxPvdTransport* transport = m_pvd->getTransport();
        m_pvd->release();
        transport->release();
    }
    PxCloseExtensions();
    m_foundation->release();
}
void PhysicsManager::init() {
    m_foundation =
        PxCreateFoundation(PX_PHYSICS_VERSION, g_default_allocator_callback,
                           g_default_error_callback);
    if (m_foundation == nullptr) {
        HL_ERROR("[Physics] A PxFoundation has already been created.");
    }

    m_pvd = PxCreatePvd(*m_foundation);
    PxPvdTransport* transport =
        PxDefaultPvdSocketTransportCreate(k_pvd_host, 5425, 10);
    m_pvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

    m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_foundation,
                                PxTolerancesScale(), true, m_pvd);
    if (m_physics == nullptr) {
        HL_ERROR("[Physics] Failed to create physics");
    }

    if (!PxInitExtensions(*m_physics, m_pvd)) {
        HL_ERROR("[Physics] PxInitExtensions failed");
    }
}

void PhysicsManager::new_scene(const SceneInfo& info) {
    if (m_scene) {
        for (auto [entity, actor] : m_actors) {
            m_scene->removeActor(*actor);
        }
        m_actors.clear();
        m_materials.clear();
    }
    m_scene_info = info;
    PxSceneDesc scene_desc(m_physics->getTolerancesScale());
    scene_desc.gravity = PxVec3(info.gravity.x, info.gravity.y, info.gravity.z);
    g_dispatcher = PxDefaultCpuDispatcherCreate(2);
    scene_desc.cpuDispatcher = g_dispatcher;
    scene_desc.filterShader = PxDefaultSimulationFilterShader;
    m_scene = m_physics->createScene(scene_desc);

    PxPvdSceneClient* pvd_client = m_scene->getScenePvdClient();
    if (pvd_client) {
        pvd_client->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS,
                                    true);
        pvd_client->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
        pvd_client->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES,
                                    true);
    }
}

void PhysicsManager::add_actor(uint32_t entity, const ActorInfo& info) {
    PxMaterial* material = m_physics->createMaterial(
        info.static_friction, info.dynamic_friction, info.restitution);

    PxTransform transform;
    transform.p = PxVec3(info.transform.position.x, info.transform.position.y,
                         info.transform.position.z);
    transform.q = PxQuat(info.transform.rotation.x, info.transform.rotation.y,
                         info.transform.rotation.z, info.transform.rotation.w);

    PxShape* shape = nullptr;
    if (info.geometry) {
        shape = m_physics->createShape(info.geometry->get_geometry().any(),
                                       *material);
    }
    PxRigidActor* actor;
    if (info.type == RigidBodyType::Dynamic) {
        actor = m_physics->createRigidDynamic(transform);
        PxRigidDynamic* dynamic = actor->is<PxRigidDynamic>();
        dynamic->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, info.kinematic);
        dynamic->setRigidDynamicLockFlags(info.lock_flags);
    } else {
        actor = m_physics->createRigidStatic(transform);
    }
    if (shape) {
        actor->attachShape(*shape);
        m_shapes[entity] = shape;
    }

    m_scene->addActor(*actor);

    m_actors[entity] = actor;
    m_materials[entity] = material;
}

void PhysicsManager::remove_actor(uint32_t entity) {
    PxActor* actor = m_actors.at(entity);
    PxShape* shape = m_shapes.at(entity);
    shape->release();
    m_scene->removeActor(*actor);
}

bool PhysicsManager::step(float ts) {
    if (m_scene) {
        m_time_acc += ts;
        if (m_time_acc < m_scene_info.timestep) {
            return false;
        }
        m_time_acc -= m_scene_info.timestep;

        m_scene->simulate(m_scene_info.timestep);
        m_scene->fetchResults(true);
        return true;
    } else {
        HL_ERROR("[Physics] No valid physics scene.");
        return false;
    }
}

void PhysicsManager::set_actor_transform(uint32_t entity,
                                         const Transform& transform) {
    PxActor* actor = m_actors.at(entity);

    switch (actor->getConcreteType()) {
    case PxConcreteType::eRIGID_DYNAMIC: {
        PxRigidDynamic* body = static_cast<PxRigidDynamic*>(actor);
        PxTransform px_transform;
        px_transform.p = PxVec3(transform.position.x, transform.position.y,
                                transform.position.z);
        px_transform.q = PxQuat(transform.rotation.x, transform.rotation.y,
                                transform.rotation.z, transform.rotation.w);

        // Setting the global pose directly
        body->setGlobalPose(px_transform);
        break;
    }
    case PxConcreteType::eRIGID_STATIC: {
        PxRigidStatic* body = static_cast<PxRigidStatic*>(actor);
        PxTransform px_transform;
        px_transform.p = PxVec3(transform.position.x, transform.position.y,
                                transform.position.z);
        px_transform.q = PxQuat(transform.rotation.x, transform.rotation.y,
                                transform.rotation.z, transform.rotation.w);

        // Static bodies are treated similarly
        body->setGlobalPose(px_transform);
        break;
    }
    default:
        HL_ERROR("[Physics] Invalid actor type.");
    }
}

Transform PhysicsManager::get_actor_transform(uint32_t entity) {
    Transform transform;
    PxActor* actor = m_actors.at(entity);

    switch (actor->getConcreteType()) {
    case PxConcreteType::eRIGID_DYNAMIC: {
        PxRigidDynamic* body = static_cast<PxRigidDynamic*>(actor);
        PxTransform px_transform = body->getGlobalPose();

        transform.position = {px_transform.p.x, px_transform.p.y,
                              px_transform.p.z};

        // Convert the quaternion correctly
        transform.rotation = glm::quat(px_transform.q.x, px_transform.q.y,
                                       px_transform.q.z, px_transform.q.w);

        break;
    }
    case PxConcreteType::eRIGID_STATIC: {
        PxRigidStatic* body = static_cast<PxRigidStatic*>(actor);
        PxTransform px_transform = body->getGlobalPose();

        transform.position = {px_transform.p.x, px_transform.p.y,
                              px_transform.p.z};
        transform.rotation = glm::quat(px_transform.q.x, px_transform.q.y,
                                       px_transform.q.z, px_transform.q.w);
        break;
    }
    default:
        HL_ERROR("[Physics] Invalid actor type.");
    }

    return transform;
}

void PhysicsManager::set_rigid_dynamic_mass(uint32_t entity, float value) {
    if (m_actors[entity]->getConcreteType() == PxConcreteType::eRIGID_DYNAMIC) {
        m_actors[entity]->is<PxRigidDynamic>()->setMass(value);
    }
}

} // namespace Helios::Physics
