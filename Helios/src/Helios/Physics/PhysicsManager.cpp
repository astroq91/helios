#include "PhysicsManager.h"
#include "Jolt/RegisterTypes.h"
#include <memory>

const uint32_t MAX_BODIES = 1024;
const uint32_t NUM_BODY_MUTEXES = 0;
const uint32_t MAX_BODY_PAIRS = 1024;
const uint32_t MAX_CONTACT_CONSTRAINTS = 1024;

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

void PhysicsManager::init() {
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
}

void PhysicsManager::set_scene(const SceneInfo& info) {
    m_scene_info = info;
    m_physics_system.SetGravity(
        {info.gravity.x, info.gravity.y, info.gravity.z});
}

bool PhysicsManager::step(float ts) { return true; }
} // namespace Helios::Physics
