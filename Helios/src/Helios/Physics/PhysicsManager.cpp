#include "PhysicsManager.h"
#include "Jolt/RegisterTypes.h"
#include <memory>

const uint32_t MAX_BODIES = 1024;
const uint32_t NUM_BODY_MUTEXES = 0;
const uint32_t MAX_BODY_PAIRS = 1024;
const uint32_t MAX_CONTACT_CONSTRAINTS = 1024;
const uint32_t COLLISION_STEPS = 1;

JPH_SUPPRESS_WARNINGS;
using namespace JPH;
using namespace JPH::literals;

static void trace_func(const char* in_fmt, ...) {
    va_list list;
    va_start(list, in_fmt);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), in_fmt, list);
    va_end(list);

    HL_TRACE(buffer);
}

#ifdef JPH_ENABLE_ASSERTS

static bool assert_failed_func(const char* in_expr, const char* in_msg,
                               const char* in_file, uint32_t in_line) {
    HL_ERROR("{}: {}: ({}) {}", in_file, in_line, in_expr,
             in_msg != nullptr ? in_msg : "");
    return true;
}

#endif

namespace Helios::Physics {
PhysicsManager::~PhysicsManager() {
    UnregisterTypes();
    delete Factory::sInstance;
    Factory::sInstance = nullptr;
}

void PhysicsManager::init() noexcept {
    // Allocation hook
    RegisterDefaultAllocator();
    Trace = trace_func;
    JPH_IF_ENABLE_ASSERTS(AssertFailed = assert_failed_func;);

    Factory::sInstance = new Factory();
    RegisterTypes();

    m_temp_allocator = std::make_unique<TempAllocatorImpl>(10 * 1024 * 1024);
    m_job_system = std::make_unique<JobSystemThreadPool>(
        cMaxPhysicsJobs, cMaxPhysicsBarriers,
        thread::hardware_concurrency() - 1);

    m_physics_system.Init(
        MAX_BODIES, NUM_BODY_MUTEXES, MAX_BODY_PAIRS, MAX_CONTACT_CONSTRAINTS,
        m_broad_phase_layer_interface, m_object_vs_broadphase_layer_filter,
        m_object_vs_object_layer_filter);

    m_physics_system.SetBodyActivationListener(&m_body_activation_listener);
    m_physics_system.SetContactListener(&m_contact_listener);

    m_physics_system.OptimizeBroadPhase();
}

void PhysicsManager::create_body(uint32_t entity,
                                 const BodyInfo& info) noexcept {
    ShapeRefC shape_ref = nullptr;
    if (std::holds_alternative<BoxShape>(info.shape)) {
        auto shape = std::get<BoxShape>(info.shape);
        BoxShapeSettings settings(
            Vec3(shape.size.x, shape.size.y, shape.size.z));
        // Unsure if this is needed
        settings.SetEmbedded();

        auto result = settings.Create();
        shape_ref = result.Get();
    } else {
        HL_ERROR("Invalid physics shape type");
    }

    EMotionType motion_type;
    ObjectLayer layer;
    if (info.type == PhysicsBodyType::Dynamic) {
        motion_type = EMotionType::Dynamic;
        layer = Layers::MOVING;
    } else if (info.type == PhysicsBodyType::Kinematic) {
        motion_type = EMotionType::Kinematic;
        layer = Layers::MOVING;
    } else if (info.type == PhysicsBodyType::Static) {
        motion_type = EMotionType::Static;
        // Possibly change this
        layer = Layers::NON_MOVING;
    } else {
        HL_ERROR("Invalid physics body type");
    }

    BodyCreationSettings body_settings(
        shape_ref,
        RVec3(info.transform.position.x, info.transform.position.y,
              info.transform.position.z),
        Quat(info.transform.rotation.x, info.transform.rotation.y,
             info.transform.rotation.z, info.transform.rotation.w),
        motion_type, layer);

    BodyID body = m_physics_system.GetBodyInterface().CreateAndAddBody(
        body_settings, EActivation::DontActivate);

    m_body_forward_map[body] = entity;
    m_body_backward_map[entity] = body;
}

void PhysicsManager::set_scene(const SceneInfo& info) noexcept {
    m_scene_info = info;
    m_physics_system.SetGravity(
        {info.gravity.x, info.gravity.y, info.gravity.z});
}

bool PhysicsManager::step(float ts) noexcept {
    m_physics_system.Update(m_scene_info.timestep, COLLISION_STEPS,
                            &*m_temp_allocator, &*m_job_system);
    return true;
}
} // namespace Helios::Physics
