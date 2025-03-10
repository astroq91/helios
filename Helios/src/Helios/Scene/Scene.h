#pragma once

#include <entt/entt.hpp>
#include <stdexcept>

#include "Helios/Renderer/Renderer.h"
#include "Helios/Scene/PerspectiveCamera.h"
#include "SceneCamera.h"
#include "entt/entity/fwd.hpp"

namespace Helios {
class Entity;
class Script;

struct SceneViewportInfo {
    Ref<Image> color_image = nullptr; 
    VkImageLayout color_image_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    glm::vec4 color_clear_value = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    Ref<Image> depth_image = nullptr; 
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

    void set_scene_camera(SceneCamera* camera) { m_scene_camera = camera; }
    void start_runtime();

    bool is_running() const { return m_runtime; }

    template <typename... Type> auto get_view() {
        return m_registry.view<Type...>();
    }

    template <typename... Type> auto get_view() const {
        return m_registry.view<Type...>();
    }

    template <typename Type> void sort_component() {
        m_registry.sort<Type>([](const entt::entity lhs,
                                 const entt::entity rhs) { return lhs < rhs; });
    }

    void update_rigid_body_mass(Entity entity, float value);
    void update_rigid_body_static_friction(Entity entity, float value);
    void update_rigid_body_dynamic_friction(Entity entity, float value);
    void update_rigid_body_restitution(Entity entity, float value);

    void render_text(const std::string& text, const glm::vec2& position,
                     float scale, const glm::vec4& tint_color);

    uint32_t get_game_viewport_width() const {
        return m_game_viewport_size.x;
    }

    uint32_t get_game_viewport_height() const {
        return m_game_viewport_size.y;
    }

  private:
    // SYSTEMS //

    void draw_systems(const PerspectiveCamera& camera);
    void update_camera(float aspect_ratio);
    void render_lighting();
    void draw_meshes();
    void update_scripts(float ts);
    void setup_signals();

    void scripting_to_physics();
    void physics_to_scripting();

    void on_rigid_body_destroyed(entt::registry& registry, entt::entity entity);

  private:
    entt::registry m_registry;
    SceneCamera* m_scene_camera;
    bool m_runtime = false;

    PerspectiveCamera m_current_camera;
    bool m_has_vaild_camera = false;

    bool m_destroyed = false;

    glm::ivec2 m_game_viewport_size;

    friend class Entity;
};
} // namespace Helios
