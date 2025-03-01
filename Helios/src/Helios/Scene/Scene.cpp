#include "Scene.h"
#include "Entity.h"
#include "Helios/Core/Application.h"
#include "Helios/Core/Core.h"
#include "Helios/ECSComponents/Components.h"
#include "Helios/Physics/PhysicsManager.h"
#include "Helios/Physics/RigidBody.h"
#include "Helios/Renderer/Renderer.h"
#include "Helios/Scene/Transform.h"

namespace Helios {

Scene::Scene(SceneCamera* sceneCamera) : m_scene_camera(sceneCamera) {
    const auto& renderer = Application::get().get_renderer();
    auto& pm = Application::get().get_physics_manager();
    pm.new_scene();

    setup_signals();
}

Scene::~Scene() { m_destroyed = true; }

Entity Scene::create_entity(const std::string& name) {
    auto enttEnt = m_registry.create();
    m_registry.emplace<NameComponent>(enttEnt, name);

    return Entity(static_cast<uint32_t>(enttEnt), this);
}

void Scene::destroy_entity(uint32_t handle) {
    m_registry.destroy(static_cast<entt::entity>(handle));
}

Entity Scene::get_entity(uint32_t handle) { return Entity(handle, this); }

void Scene::on_update(float ts, const BeginRenderingSpec& editor_spec,
                      const BeginRenderingSpec& game_spec) {
    if (m_runtime) {
        update_scripts(ts);
        scripting_to_physics();
        Application::get().get_physics_manager().step(ts);
        physics_to_scripting();
    }

    auto& renderer = Application::get().get_renderer();

    // Editor viewport
    if (m_scene_camera) {
        draw_systems(m_scene_camera->get_camera());
    }
    renderer.submit_instances(editor_spec);
    // Need to submit because we need to use different cameras (camera is a
    // uniform)
    renderer.submit_command_buffer();

    update_camera(static_cast<float>(game_spec.width) /
                  static_cast<float>(game_spec.height));
    // Game viewport
    if (m_has_vaild_camera) {
        draw_systems(m_current_camera);
    }
    renderer.submit_instances(game_spec);
}

void Scene::update_rigid_body_mass(Entity entity, float value) {
    RigidBodyComponent* rb = entity.try_get_component<RigidBodyComponent>();
    if (m_runtime && rb && rb->type == RigidBodyType::Dynamic) {
        auto& pm = Application::get().get_physics_manager();
        pm.set_rigid_dynamic_mass(entity, value);
    }
}

void Scene::update_rigid_body_static_friction(Entity entity, float value) {
    if (m_runtime && entity.has_component<RigidBodyComponent>()) {
        auto& pm = Application::get().get_physics_manager();
        pm.set_material_static_friction(entity, value);
    }
}
void Scene::update_rigid_body_dynamic_friction(Entity entity, float value) {
    if (m_runtime && entity.has_component<RigidBodyComponent>()) {
        auto& pm = Application::get().get_physics_manager();
        pm.set_material_dynamic_friction(entity, value);
    }
}
void Scene::update_rigid_body_restitution(Entity entity, float value) {
    if (m_runtime && entity.has_component<RigidBodyComponent>()) {
        auto& pm = Application::get().get_physics_manager();
        pm.set_material_restitution(entity, value);
    }
}

void Scene::draw_systems(const Camera& camera) {
    auto& renderer = Application::get().get_renderer();
    renderer.set_camera(camera);

    render_lighting();
    draw_meshes();
}

void Scene::update_camera(float aspect_ratio) {
    auto& renderer = Application::get().get_renderer();

    m_has_vaild_camera = true;

    auto camera_view =
        m_registry.view<const TransformComponent, const CameraComponent>();

    // If no scene camera, or ecs camera then there is no valid camera
    if (camera_view.begin() == camera_view.end()) {
        m_has_vaild_camera = false;
    }

    for (auto [entity, transform, cam] : camera_view.each()) {
        m_current_camera = Camera(transform.to_transform(), aspect_ratio,
                                  cam.fovY, cam.z_near, cam.z_far);
        break;
    }
}

void Scene::render_lighting() {
    auto& renderer = Application::get().get_renderer();

    auto directional_lights_view =
        m_registry.view<const DirectionalLightComponent>();
    for (auto [entity, dir_light] : directional_lights_view.each()) {
        renderer.render_directional_light(dir_light.to_directional_light());
    }

    auto point_lights_view = m_registry.view<const PointLightComponent>();
    for (auto [entity, point_light] : point_lights_view.each()) {
        renderer.render_point_light(point_light.to_point_light());
    }
}

void Scene::draw_meshes() {
    auto& renderer = Application::get().get_renderer();

    auto meshes_view =
        m_registry.view<const TransformComponent, const MeshRendererComponent>();

    std::unordered_map<uuids::uuid, std::vector<MeshRenderingInstance>> mesh_groups;
    std::unordered_map<uuids::uuid, Ref<Mesh>> meshes;

    for (auto [entity, transform, mesh] : meshes_view.each()) {
        if (mesh.mesh == nullptr) {
            continue;
        }

        mesh_groups[mesh.mesh->get_uuid()].push_back(
            {.transform = transform.to_transform(),
             .material = mesh.material,
             .tint_color = mesh.tint_color});
        meshes[mesh.mesh->get_uuid()] = mesh.mesh;
    }

    for (auto& [id, mesh] : meshes) {
        renderer.draw_mesh(mesh, mesh_groups[id]);
    }
}

void Scene::update_scripts(float ts) {
    auto scripts_view = m_registry.view<ScriptComponent>();
    for (auto [entity, script] : scripts_view.each()) {
        if (script.script != nullptr) {
            script.script->on_update(ts);
        }
    }
}

void Scene::start_runtime() {
    auto& app = Application::get();
    auto& pm = app.get_physics_manager();

    auto scripts_view = m_registry.view<ScriptComponent>();
    for (auto [entity, script] : scripts_view.each()) {
        if (script.script != nullptr) {
            script.script->on_start();
        }
    }

    auto rb_view =
        m_registry.view<const TransformComponent, const RigidBodyComponent>();
    for (auto [entity, transform, rb] : rb_view.each()) {
        Physics::Geometry geom = Physics::BoxGeometry();
        bool use_geometry = false;

        BoxColliderComponent* bc =
            m_registry.try_get<BoxColliderComponent>(entity);
        if (bc) {
            geom = Physics::BoxGeometry(bc->size);
            use_geometry = true;
        }

        pm.add_actor(static_cast<uint32_t>(entity),
                     {
                         .type = rb.type,
                         .geometry = use_geometry ? &geom : nullptr,
                         .transform = {},
                         .static_friction = rb.static_friction,
                         .dynamic_friction = rb.dynamic_friction,
                         .restitution = rb.restitution,
                     });
        pm.set_actor_transform(static_cast<uint32_t>(entity),
                               transform.to_transform());
    }

    m_runtime = true;
}

void Scene::setup_signals() {
    m_registry.on_destroy<RigidBodyComponent>()
        .connect<&Scene::on_rigid_body_destroyed>(*this);
}

void Scene::scripting_to_physics() {
    auto& pm = Application::get().get_physics_manager();
    auto rigid_body_view =
        m_registry.view<const TransformComponent, const RigidBodyComponent>();
    for (auto [entity, transform, rb] : rigid_body_view.each()) {
        if (rb.type == RigidBodyType::Static || rb.kinematic ||
            rb.override_dynamic_physics) {
            pm.set_actor_transform(static_cast<uint32_t>(entity),
                                   transform.to_transform());
        }
    }
}

void Scene::physics_to_scripting() {
    auto& pm = Application::get().get_physics_manager();
    auto rigid_body_view =
        m_registry.view<TransformComponent, const RigidBodyComponent>();
    for (auto [entity, transform, rb] : rigid_body_view.each()) {
        Transform t = pm.get_actor_transform(static_cast<uint32_t>(entity));
        transform.position = t.position;
        transform.rotation = t.rotation;
    }
}

void Scene::on_rigid_body_destroyed(entt::registry& registry,
                                    entt::entity entity) {
    if (m_runtime && !m_destroyed) {
        auto& pm = Application::get().get_physics_manager();

        auto box_collider = registry.try_get<BoxColliderComponent>(entity);
        if (box_collider) {
            registry.remove<BoxColliderComponent>(entity);
        }
        pm.remove_actor(static_cast<uint32_t>(entity));
    }
}

} // namespace Helios
