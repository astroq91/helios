#pragma once

#include <chrono>
#include <entt/entt.hpp>
#include <unordered_map>

#include "Helios/Renderer/Renderer.h"
#include "Helios/Scene/PerspectiveCamera.h"
#include "SceneCamera.h"
#include "entt/entity/fwd.hpp"

namespace Helios {
constexpr uint32_t k_no_entity = UINT32_MAX;

class Entity;
class Script;

struct SceneViewportInfo {
    SharedPtr<Image> color_image = nullptr;
    VkImageLayout color_image_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    glm::vec4 color_clear_value = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    SharedPtr<Image> depth_image = nullptr;
    uint32_t width = 0;
    uint32_t height = 0;
};

class Scene {
  public:
    /**
     * \brief create a new scene.
     */
    Scene(SceneCamera* sceneCamera = nullptr);

    ~Scene();

    void scene_load_done();

    /**
     * \brief Creates a new entity.
     * \param name The name of the entity.
     * \return
     */
    Entity create_entity(const std::string& name);

    void destroy_entity(uint32_t handle);

    Entity get_entity(uint32_t handle);

    /**
     * \brief Call to handle all default ECS systems.
     * \param ts The current timestep.
     */
    void on_update(float ts, const SceneViewportInfo& editor_spec,
                   const SceneViewportInfo& game_spec);

    void on_fixed_update();

    void set_scene_camera(SceneCamera* camera) { m_scene_camera = camera; }
    void start_runtime();

    bool is_running() const { return m_runtime; }

    template <typename... Type, typename... Exclude>
    auto get_view(entt::exclude_t<Exclude...> = entt::exclude_t{}) {
        return m_registry.view<Type...>(entt::exclude<Exclude...>);
    }

    template <typename... Type> auto get_view() const {
        return m_registry.view<Type...>();
    }

    template <typename Type> void sort_component() {
        m_registry.sort<Type>([](const entt::entity lhs,
                                 const entt::entity rhs) { return lhs < rhs; });
    }

    void update_physics_body_gravity_factor(Entity entity, float value);
    void update_physics_body_friction(Entity entity, float value);
    void update_physics_body_restitution(Entity entity, float value);

    void on_entity_transform_updated(Entity entity);
    // TODO?: void on_entity_local_transform_updated(Entity entity);

    void on_entity_position_updated(Entity entity);
    void on_entity_rotation_updated(Entity entity);
    void on_entity_scale_updated(Entity entity);
    void on_entity_local_position_updated(Entity entity);
    void on_entity_local_rotation_updated(Entity entity);
    void on_entity_local_scale_updated(Entity entity);

    void render_text(const std::string& text, const glm::vec2& position,
                     float scale, const glm::vec4& tint_color);

    uint32_t get_game_viewport_width() const { return m_game_viewport_size.x; }

    uint32_t get_game_viewport_height() const { return m_game_viewport_size.y; }

    const std::vector<uint32_t>* const
    try_get_entity_children(Entity entity) const;

    uint32_t get_entity_from_uuid(const uuids::uuid& id) const {
        if (m_entity_id_map.contains(id)) {
            return m_entity_id_map.at(id);
        }
        return k_no_entity;
    }

    bool is_game_viewport_focused() const { return m_game_viewport_focused; }
    void set_game_viewport_focused(bool selected) {
        m_game_viewport_focused = selected;
    }

    const glm::vec2& get_game_viewport_position() const {
        return m_game_viewport_position;
    }
    void set_game_viewport_position(const glm::vec2& position) {
        m_game_viewport_position = position;
    }

    void set_skybox_enabled(bool use_skybox) { m_skybox_enabled = use_skybox; }
    void set_custom_skybox(const SharedPtr<Texture>& skybox);

    bool skybox_enabled() const { return m_skybox_enabled; }
    bool has_custom_skybox() const { return m_custom_skybox; }
    const SharedPtr<Texture>& get_skybox() const { return m_skybox; }

  private:
    // SYSTEMS //

    void draw_systems(const PerspectiveCamera& camera);
    void update_camera(float aspect_ratio);
    void render_lighting();
    void draw_meshes();
    void update_scripts(float ts);
    void update_scripts_fixed();
    void setup_signals();

    void scripting_to_physics();
    void physics_to_scripting();

    void on_rigid_body_destroyed(entt::registry& registry, entt::entity entity);
    void on_parent_component_added(entt::registry& registry,
                                   entt::entity entity);
    void on_parent_component_destroyed(entt::registry& registry,
                                       entt::entity entity);
    void on_persistent_id_component_added(entt::registry& registry,
                                          entt::entity entity);
    void on_persistent_id_component_destroyed(entt::registry& registry,
                                              entt::entity entity);
    void update_children();

    void create_custom_pipelines();

    void set_skybox(const SharedPtr<Texture>& skybox);

  private:
    entt::registry m_registry;
    SceneCamera* m_scene_camera;
    bool m_runtime = false;

    PerspectiveCamera m_current_camera;
    bool m_has_vaild_camera = false;

    bool m_destroyed = false;

    std::unordered_map<uint32_t, std::vector<uint32_t>> m_entity_children;
    std::unordered_map<uuids::uuid, uint32_t> m_entity_id_map;

    std::unordered_map<SharedPtr<Material>, SharedPtr<Pipeline>>
        m_custom_pipelines;

    glm::ivec2 m_game_viewport_size;
    glm::vec2 m_game_viewport_position;
    bool m_game_viewport_focused = false;

    std::chrono::time_point<std::chrono::high_resolution_clock> m_start_time;

    bool m_skybox_enabled = true;
    bool m_custom_skybox = false;
    SharedPtr<Texture> m_skybox = nullptr;

    friend class Entity;
};
} // namespace Helios
