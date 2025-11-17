#include "Scene.h"
#include "Entity.h"
#include "Helios/Core/Application.h"
#include "Helios/Core/Core.h"
#include "Helios/ECSComponents/Components.h"
#include "Helios/Physics/PhysicsManager.h"
#include "Helios/Renderer/Renderer.h"
#include "Helios/Scene/Transform.h"
#include "stduuid/uuid.h"
#include "vulkan/vulkan_core.h"
#include <chrono>
#include <cstring>
#include <variant>

struct ShaderPushConstantsData {
    alignas(4) float time;
    alignas(8) glm::vec2 resolution;
    alignas(8) glm::vec2 mouse;
};

namespace Helios {

Scene::Scene(SceneCamera* sceneCamera) : m_scene_camera(sceneCamera) {
    const auto& renderer = Application::get().get_renderer();
    auto& pm = Application::get().get_physics_manager();
    pm.set_scene({});

    setup_signals();
    m_start_time = std::chrono::high_resolution_clock::now();

    set_skybox(
        Application::get().get_asset_manager().get_texture("default_skybox"));
}

Scene::~Scene() {
    m_destroyed = true;
    auto& pm = Application::get().get_physics_manager();
    pm.destroy_bodies();
}

void Scene::scene_load_done() {
    auto view = m_registry.view<ScriptComponent>();

    for (auto [entity, script] : view.each()) {
        for (auto& field : script.exposed_fields) {
            ScriptField* script_field =
                script.script->get_exposed_field(field.name);

            if (script_field) {
                switch (script_field->get_type()) {
                case ScriptFieldType::Entity: {
                    if (std::holds_alternative<uuids::uuid>(field.value)) {
                        const uuids::uuid& id =
                            std::get<uuids::uuid>(field.value);
                        if (m_entity_id_map.contains(id)) {
                            uint32_t entity_id = m_entity_id_map.at(id);
                            script_field->set_value(entity_id);
                        }
                    }
                    break;
                }
                }
            }
        }
    }
}

Entity Scene::create_entity(const std::string& name) {
    auto enttEnt = m_registry.create();
    m_registry.emplace<NameComponent>(enttEnt, name);

    return Entity(static_cast<uint32_t>(enttEnt), this);
}

void Scene::destroy_entity(uint32_t handle) {
    m_registry.destroy(static_cast<entt::entity>(handle));
}

Entity Scene::get_entity(uint32_t handle) {
    // Check if the entity exists (has a name component)
    if (m_registry.try_get<NameComponent>(static_cast<entt::entity>(handle))) {
        return Entity(handle, this);
    }
    return Entity();
}

void Scene::on_update(float ts, const SceneViewportInfo& editor_spec,
                      const SceneViewportInfo& game_spec) {
    auto& renderer = Application::get().get_renderer();

    if (m_runtime) {
        m_game_viewport_size = {game_spec.width, game_spec.height};
        renderer.set_ui_projection_matrix(glm::orthoRH_ZO(
            0.0f, static_cast<float>(game_spec.width),
            static_cast<float>(game_spec.height), 0.0f, 0.0f, 1.0f));

        update_scripts(ts);
        scripting_to_physics();
        Application::get().get_physics_manager().step(ts);
        physics_to_scripting();
    }

    update_children();

    // Editor viewport
    if (m_scene_camera) {
        draw_systems(m_scene_camera->get_camera());
    }
    renderer.update_camera_uniform();
    if (m_skybox_enabled && m_skybox) {

        renderer.render_skybox({
            .color_image = editor_spec.color_image,
            .color_image_layout = editor_spec.color_image_layout,
            .color_load_op = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .color_store_op = VK_ATTACHMENT_STORE_OP_STORE,
            .color_clear_value = editor_spec.color_clear_value,
            .depth_image = editor_spec.depth_image,
            .width = editor_spec.width,
            .height = editor_spec.height,
        });
    }

    renderer.submit_mesh_instances({
        .color_image = editor_spec.color_image,
        .color_image_layout = editor_spec.color_image_layout,
        // Only clear if we are not using a skybox
        .color_load_op = m_skybox_enabled ? VK_ATTACHMENT_LOAD_OP_LOAD
                                          : VK_ATTACHMENT_LOAD_OP_CLEAR,
        .color_store_op = VK_ATTACHMENT_STORE_OP_STORE,
        .color_clear_value = editor_spec.color_clear_value,
        .depth_image = editor_spec.depth_image,
        .width = editor_spec.width,
        .height = editor_spec.height,
    });
    // Need to submit because we need to use different cameras (camera is a
    // uniform)
    renderer.submit_command_buffer();

    update_camera(static_cast<float>(game_spec.width) /
                  static_cast<float>(game_spec.height));
    // Game viewport
    if (m_has_vaild_camera) {
        draw_systems(m_current_camera);
    }
    renderer.update_camera_uniform();
    renderer.submit_mesh_instances({
        .color_image = game_spec.color_image,
        .color_image_layout = game_spec.color_image_layout,
        .color_load_op = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .color_store_op = VK_ATTACHMENT_STORE_OP_STORE,
        .color_clear_value = game_spec.color_clear_value,
        .depth_image = game_spec.depth_image,
        .width = game_spec.width,
        .height = game_spec.height,
    });

    // Need to submit because mesh instancing, and ui instancing, share staging
    // buffers
    renderer.submit_command_buffer();

    renderer.submit_ui_quad_instances({
        .color_image = game_spec.color_image,
        .color_image_layout = game_spec.color_image_layout,
        .color_load_op = VK_ATTACHMENT_LOAD_OP_LOAD,
        .color_store_op = VK_ATTACHMENT_STORE_OP_STORE,
        .depth_image = game_spec.depth_image,
        .width = game_spec.width,
        .height = game_spec.height,
    });

    renderer.submit_command_buffer();
}

void Scene::on_fixed_update() {
    if (!m_runtime) {
        return;
    }

    update_scripts_fixed();
}

void Scene::update_physics_body_gravity_factor(Entity entity, float value) {
    PhysicsBodyComponent* body =
        entity.try_get_component<PhysicsBodyComponent>();
    if (m_runtime && body && body->type == PhysicsBodyType::Dynamic) {
        auto& pm = Application::get().get_physics_manager();
        pm.set_gravity_factor(entity, value);
    }
}

void Scene::update_physics_body_friction(Entity entity, float value) {
    if (m_runtime && entity.has_component<PhysicsBodyComponent>()) {
        auto& pm = Application::get().get_physics_manager();
        pm.set_friction(entity, value);
    }
}
void Scene::update_physics_body_restitution(Entity entity, float value) {
    if (m_runtime && entity.has_component<PhysicsBodyComponent>()) {
        auto& pm = Application::get().get_physics_manager();
        pm.set_restitution(entity, value);
    }
}

void Scene::on_entity_transform_updated(Entity entity) {
    on_entity_position_updated(entity);
    on_entity_rotation_updated(entity);
    on_entity_scale_updated(entity);
}

void Scene::on_entity_position_updated(Entity entity) {

    bool local_transform_relative_to_world = false;
    auto transform = entity.try_get_component<TransformComponent>();
    if (transform) {
        auto parent_component = entity.try_get_component<ParentComponent>();
        if (parent_component) {
            auto parent = get_entity(parent_component->parent);

            auto parent_transform =
                parent.try_get_component<TransformComponent>();
            if (parent_transform) {

                transform->local_position =
                    transform->position - parent_transform->position;
            } else {
                local_transform_relative_to_world = true;
            }
        } else {
            local_transform_relative_to_world = true;
        }
    }

    if (local_transform_relative_to_world) {
        transform->local_position = transform->position;
    }
}
void Scene::on_entity_rotation_updated(Entity entity) {

    bool local_transform_relative_to_world = false;
    auto transform = entity.try_get_component<TransformComponent>();
    if (transform) {
        auto parent_component = entity.try_get_component<ParentComponent>();
        if (parent_component) {
            auto parent = get_entity(parent_component->parent);

            auto parent_transform =
                parent.try_get_component<TransformComponent>();
            if (parent_transform) {

                transform->local_rotation =
                    glm::inverse(parent_transform->rotation) *
                    transform->rotation;
            } else {
                local_transform_relative_to_world = true;
            }
        } else {
            local_transform_relative_to_world = true;
        }
    }

    if (local_transform_relative_to_world) {
        transform->local_rotation = transform->rotation;
    }
}
void Scene::on_entity_scale_updated(Entity entity) {

    bool local_transform_relative_to_world = false;
    auto transform = entity.try_get_component<TransformComponent>();
    if (transform) {
        auto parent_component = entity.try_get_component<ParentComponent>();
        if (parent_component) {
            auto parent = get_entity(parent_component->parent);

            auto parent_transform =
                parent.try_get_component<TransformComponent>();
            if (parent_transform) {

                transform->local_scale =
                    transform->scale / parent_transform->scale;
            } else {
                local_transform_relative_to_world = true;
            }
        } else {
            local_transform_relative_to_world = true;
        }
    }

    if (local_transform_relative_to_world) {
        transform->local_scale = transform->scale;
    }
}

void Scene::on_entity_local_position_updated(Entity entity) {

    bool local_transform_relative_to_world = false;
    auto transform = entity.try_get_component<TransformComponent>();
    if (transform) {
        auto parent_component = entity.try_get_component<ParentComponent>();
        if (parent_component) {
            auto parent = get_entity(parent_component->parent);

            auto parent_transform =
                parent.try_get_component<TransformComponent>();
            if (parent_transform) {

                transform->position =
                    parent_transform->position + transform->local_position;
            } else {
                local_transform_relative_to_world = true;
            }
        } else {
            local_transform_relative_to_world = true;
        }
    }

    if (local_transform_relative_to_world) {
        transform->position = transform->local_position;
    }
}
void Scene::on_entity_local_rotation_updated(Entity entity) {
    bool local_transform_relative_to_world = false;
    auto transform = entity.try_get_component<TransformComponent>();
    if (transform) {
        auto parent_component = entity.try_get_component<ParentComponent>();
        if (parent_component) {
            auto parent = get_entity(parent_component->parent);

            auto parent_transform =
                parent.try_get_component<TransformComponent>();
            if (parent_transform) {

                transform->rotation =
                    parent_transform->rotation * transform->local_rotation;
            } else {
                local_transform_relative_to_world = true;
            }
        } else {
            local_transform_relative_to_world = true;
        }
    }

    if (local_transform_relative_to_world) {
        transform->rotation = transform->local_rotation;
    }
}
void Scene::on_entity_local_scale_updated(Entity entity) {
    bool local_transform_relative_to_world = false;
    auto transform = entity.try_get_component<TransformComponent>();
    if (transform) {
        auto parent_component = entity.try_get_component<ParentComponent>();
        if (parent_component) {
            auto parent = get_entity(parent_component->parent);

            auto parent_transform =
                parent.try_get_component<TransformComponent>();
            if (parent_transform) {

                transform->scale =
                    parent_transform->scale * transform->local_scale;
            } else {
                local_transform_relative_to_world = true;
            }
        } else {
            local_transform_relative_to_world = true;
        }
    }

    if (local_transform_relative_to_world) {
        transform->scale = transform->local_scale;
    }
}
void Scene::render_text(const std::string& text, const glm::vec2& position,
                        float scale, const glm::vec4& tint_color) {
    auto& renderer = Application::get().get_renderer();
    renderer.render_text(text, position, scale, tint_color);
}

void Scene::draw_systems(const PerspectiveCamera& camera) {
    auto& renderer = Application::get().get_renderer();
    renderer.set_perspective_camera(camera);

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
        m_current_camera =
            PerspectiveCamera(transform.to_transform(), aspect_ratio, cam.fovY,
                              cam.z_near, cam.z_far);
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
        m_registry
            .view<const TransformComponent, const MeshRendererComponent>();

    std::map<uuids::uuid, std::vector<MeshRenderingInstance>> mesh_groups;
    std::map<uuids::uuid, SharedPtr<Mesh>> meshes;

    std::vector<
        std::tuple<SharedPtr<Mesh>, MeshRenderingInstance, SharedPtr<Pipeline>>>
        meshes_custom_shaders;

    for (auto [entity, transform, mesh] : meshes_view.each()) {
        if (mesh.mesh == nullptr) {
            continue;
        }

        // Custom shaders
        if (mesh.material && (mesh.material->get_vertex_shader() ||
                              mesh.material->get_fragment_shader())) {
            SharedPtr<Pipeline> custom_pipeline = nullptr;
            if (m_custom_pipelines.contains(mesh.material)) {
                custom_pipeline = m_custom_pipelines.at(mesh.material);
            } else {
                custom_pipeline = Pipeline::create({
                    renderer.get_swapchain()->get_vk_format(),
                    {
                        renderer.get_camera_uniform_set_layout(),
                        renderer.get_texture_array_layout(),
                    },
                    mesh.material->get_vertex_shader()
                        ? mesh.material->get_vertex_shader()
                        : renderer.get_lighting_vertex_shader(),
                    mesh.material->get_fragment_shader()
                        ? mesh.material->get_fragment_shader()
                        : renderer.get_lighting_fragment_shader(),
                    {renderer.get_meshes_vertices_description(),
                     renderer
                         .get_mesh_rendering_instance_vertices_description()},
                    {
                        VkPushConstantRange{
                            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                            .offset = 0,
                            .size = sizeof(ShaderPushConstantsData)},
                    },
                });

                m_custom_pipelines.insert(
                    std::pair<SharedPtr<Material>, SharedPtr<Pipeline>>(
                        mesh.material, custom_pipeline));
            }

            meshes_custom_shaders.push_back(
                std::tuple<SharedPtr<Mesh>, MeshRenderingInstance,
                           SharedPtr<Pipeline>>(
                    mesh.mesh,
                    {
                        .transform = transform.to_transform(),
                        .material = mesh.material,
                        .tint_color = mesh.tint_color,

                    },
                    custom_pipeline));
        } else {
            mesh_groups[mesh.mesh->get_uuid()].push_back({
                .transform = transform.to_transform(),
                .material = mesh.material,
                .tint_color = mesh.tint_color,
            });
            meshes[mesh.mesh->get_uuid()] = mesh.mesh;
        }
    }

    for (auto& [id, mesh] : meshes) {
        renderer.draw_mesh(mesh, mesh_groups[id]);
    }

    const auto& window = Application::get().get_window();
    Application::get().get_window().get_width();

    auto duration = std::chrono::high_resolution_clock::now() - m_start_time;
    ShaderPushConstantsData push_constants{
        .time = std::chrono::duration_cast<std::chrono::milliseconds>(duration)
                    .count() /
                1000.f,
        .resolution = {window.get_width(), window.get_height()},
        .mouse = Input::get_mouse_pos(),
    };

    std::vector<uint8_t> push_constants_data(sizeof(ShaderPushConstantsData));
    std::memcpy(push_constants_data.data(), &push_constants,
                sizeof(ShaderPushConstantsData));

    for (auto& [mesh, info, pipeline] : meshes_custom_shaders) {
        renderer.draw_mesh(mesh, {info},
                           {.pipeline = pipeline,
                            .descriptor_sets =
                                {
                                    renderer.get_current_camera_uniform_set(),
                                    renderer.get_current_texture_array(),
                                },
                            .push_constants = {
                                .size = sizeof(ShaderPushConstantsData),
                                .stages = VK_SHADER_STAGE_FRAGMENT_BIT,
                                .data = push_constants_data,
                            }});
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

void Scene::update_scripts_fixed() {
    auto scripts_view = m_registry.view<ScriptComponent>();
    for (auto [entity, script] : scripts_view.each()) {
        if (script.script != nullptr) {
            script.script->on_fixed_update();
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

    auto pb_view =
        m_registry.view<const TransformComponent, const PhysicsBodyComponent>(
            entt::exclude<ParentComponent>);
    for (auto [entity, transform, pb] : pb_view.each()) {
        pm.create_body(static_cast<uint32_t>(entity),
                       Physics::BodyInfo{
                           .type = pb.type,
                           .shape = pb.shape,
                           .transform = transform.to_transform(),
                           .friction = pb.friction,
                           .restitution = pb.restitution,
                       });
    }

    m_runtime = true;
}

const std::vector<uint32_t>* const
Scene::try_get_entity_children(Entity entity) const {
    if (m_entity_children.contains(entity)) {
        return &m_entity_children.find(entity)->second;
    }
    return nullptr;
}

void Scene::set_custom_skybox(const SharedPtr<Texture>& skybox) {
    m_custom_skybox = skybox == true;

    // If the skybox is nullptr, and it's enabled, use the default skybox
    if (!m_custom_skybox && m_skybox_enabled) {
        set_skybox(Application::get().get_asset_manager().get_texture(
            "default_skybox"));
    } else {
        set_skybox(skybox);
    }
}

void Scene::set_skybox(const SharedPtr<Texture>& skybox) {
    m_skybox = skybox;
    if (m_skybox) {
        Application::get().get_renderer().set_skybox(m_skybox);
    }
}

void Scene::setup_signals() {
    m_registry.on_construct<PersistentIdComponent>()
        .connect<&Scene::on_persistent_id_component_added>(*this);
    m_registry.on_destroy<PersistentIdComponent>()
        .connect<&Scene::on_persistent_id_component_destroyed>(*this);
    m_registry.on_destroy<PhysicsBodyComponent>()
        .connect<&Scene::on_physics_body_destroyed>(*this);
    m_registry.on_construct<ParentComponent>()
        .connect<&Scene::on_parent_component_added>(*this);
    m_registry.on_destroy<ParentComponent>()
        .connect<&Scene::on_parent_component_destroyed>(*this);
}

void Scene::scripting_to_physics() {
    auto& pm = Application::get().get_physics_manager();
    auto rigid_body_view =
        m_registry.view<const TransformComponent, const PhysicsBodyComponent>(
            entt::exclude<ParentComponent>);
    for (auto [entity, transform, pb] : rigid_body_view.each()) {
        if (pb.type == PhysicsBodyType::Static ||
            pb.type == PhysicsBodyType::Kinematic ||
            pb.override_dynamic_physics) {
            pm.set_transform(static_cast<uint32_t>(entity),
                             transform.to_transform());
        }
    }
}

void Scene::physics_to_scripting() {
    auto& pm = Application::get().get_physics_manager();
    auto rigid_body_view =
        m_registry.view<TransformComponent, const PhysicsBodyComponent>(
            entt::exclude<ParentComponent>);
    for (auto [entity, transform, pb] : rigid_body_view.each()) {
        Transform t = pm.get_transform(static_cast<uint32_t>(entity));
        transform.position = t.position;
        transform.rotation = t.rotation;
    }
}

void Scene::on_physics_body_destroyed(entt::registry& registry,
                                      entt::entity entity) {
    if (m_runtime && !m_destroyed) {
        auto& pm = Application::get().get_physics_manager();

        pm.destroy_body(static_cast<uint32_t>(entity));
    }
}

void Scene::on_parent_component_added(entt::registry& registry,
                                      entt::entity entity) {
    auto& pc = registry.get<ParentComponent>(entity);
    m_entity_children.try_emplace(pc.parent, std::vector<uint32_t>());
    auto& vec = m_entity_children.at(pc.parent);
    vec.push_back(static_cast<uint32_t>(entity));
}

void Scene::on_parent_component_destroyed(entt::registry& registry,
                                          entt::entity entity) {
    auto& pc = registry.get<ParentComponent>(entity);
    auto& vec = m_entity_children.at(pc.parent);
    for (size_t i = 0; i < vec.size(); i++) {
        if (vec[i] == static_cast<uint32_t>(entity)) {
            vec.erase(vec.begin() + i);
        }
    }
}

void Scene::on_persistent_id_component_added(entt::registry& registry,
                                             entt::entity entity) {
    auto& pic = registry.get<PersistentIdComponent>(entity);
    m_entity_id_map.insert(std::pair<uuids::uuid, uint32_t>(
        pic.get_id(), static_cast<uint32_t>(entity)));
}

void Scene::on_persistent_id_component_destroyed(entt::registry& registry,
                                                 entt::entity entity) {
    auto& pic = registry.get<PersistentIdComponent>(entity);
    m_entity_id_map.erase(pic.get_id());
}

void Scene::update_children() {
    auto view = m_registry.view<TransformComponent, const ParentComponent>();
    for (auto [entity, transform, parent] : view.each()) {
        Entity parent_ent = get_entity(parent.parent);
        if (parent_ent != k_no_entity &&
            parent_ent.has_component<TransformComponent>()) {
            auto& parent_transform =
                parent_ent.get_component<const TransformComponent>();
            transform.position =
                parent_transform.position + transform.local_position;
            transform.rotation =
                parent_transform.rotation * transform.local_rotation;
            transform.scale = parent_transform.scale * transform.local_scale;
        }
    }
}

} // namespace Helios
