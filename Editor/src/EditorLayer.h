#pragma once

#include "Helios/Scene/Entity.h"
#include "Project.h"

#include "Helios/Core/Core.h"
#include "Helios/ECSComponents/Components.h"
#include <Helios/Core/Layer.h>

#include <Helios/Scene/Camera.h>

#include <imgui.h>

#include "ComponentsBrowser.h"
#include "AssetsBrowser.h"
#include "Helios/Scene/Scene.h"
#include "Helios/Scene/SceneCamera.h"
#include "ImGuizmo.h"

#include <chrono>

// Used as per-instance vertex attributes for an entity, in the shader.
struct EntityPickingShaderData {
    alignas(16) glm::mat4 model;
    alignas(4) uint32_t entity_id;
};

// Used to keep track of a matching entity's mesh, from EntityPickingShaderData.
struct EntityPickingData {
    Helios::Ref<Helios::Mesh> mesh;
};

struct ViewportData {
    std::vector<Helios::Ref<Helios::Image>>
        images; // One for each frame in flight
    std::vector<Helios::Ref<Helios::Image>>
        depth_images;                     // One for each frame in flight
    std::vector<VkDescriptorSet> handles; // One for each frame in flight
    Helios::Unique<Helios::TextureSampler> sampler;

    ImVec2 size;
    ImVec2 mouse_pos;

    bool focused = false;
    bool hovered = false;
};

struct NewSceneInfo {
    bool reset_selected_entity = true;
    bool reset_scene_camera = true;
    bool reset_scene_path = true;
    bool reset_window_title = true;
};

class EditorLayer : public Helios::Layer {
  public:
    EditorLayer();
    ~EditorLayer() override;

    void on_attach() override;
    void on_detach() override;

    void on_update(float ts) override;
    void on_event(Helios::Event& e) override;
    void on_imgui_render() override;

  private:
    void setup_editor_grid();
    void populate_viewport_data(ViewportData& viewport);
    void render_editor_grid();
    void update_entity_picking();
    bool viewport_resized(const ViewportData& viewport);
    void resize_viewport(ViewportData& viewport);
    void update_scene_camera_uniform();
    void create_or_recreate_picking_images();
    void setup_entity_picking();
    void select_entity(uint32_t entity);
    void new_scene(const NewSceneInfo& info = {});
    void save_scene(const std::string& path);
    void load_scene(const std::filesystem::path& path);
    void show_welcome_window();
    void show_new_project_window();
    void update_window_title(const std::optional<std::string>& scene_path);
    void reset_editor();
    void stop_runtime();

  private:
    std::optional<Project> m_project;
    bool m_show_new_project_window = false;

    Helios::SceneCamera m_scene_camera;

    Helios::Scene* m_scene = nullptr;

    std::optional<std::filesystem::path> m_loaded_scene_path;

    ViewportData m_editor_viewport;
    ViewportData m_game_viewport;

    Helios::Entity m_selected_entity;
    Helios::TransformComponent* m_selected_entity_transform = nullptr;

    Helios::ComponentsBrowser m_components_browser;
    Helios::AssetsBrowser m_assets_browser;

    // Scene camera //
    std::vector<Helios::Ref<Helios::Buffer>> m_scene_camera_uniform_buffers;
    Helios::Ref<Helios::DescriptorSetLayout> m_scene_camera_set_layout;
    std::vector<Helios::Unique<Helios::DescriptorSet>>
        m_scene_camera_descriptor_sets;
    Helios::Ref<Helios::DescriptorPool> m_scene_camera_set_pool;

    // Entity picking //
    Helios::Unique<Helios::Pipeline> m_entity_picking_pipeline;
    std::vector<Helios::Ref<Helios::VertexBuffer>> m_entity_picking_buffers;
    // Maybe use multi-buffering for these as well?
    std::vector<EntityPickingShaderData> m_entity_picking_shader_data;
    std::vector<Helios::Ref<Helios::Image>>
        m_entity_picking_images; // One for each frame in flight
    std::vector<Helios::Ref<Helios::Image>>
        m_entity_picking_depth_images; // One for each frame in flight
    Helios::Ref<Helios::Buffer> m_entity_picking_staging_buffer;
    bool m_use_mouse_picking = false;

    // Editor grid //
    Helios::Unique<Helios::Pipeline> m_grid_pipeline;

    ImGuizmo::OPERATION m_gizmo_operation = ImGuizmo::OPERATION::TRANSLATE;
    ImGuizmo::MODE m_gizmo_mode = ImGuizmo::MODE::LOCAL;
    bool m_using_gizmo = false;
    bool m_use_snap = false;

    bool m_go_to_homescreen = false;

    uint32_t m_fps = 0;
    uint32_t m_frame_counter = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_fps_clock;
    double m_fps_sampling_rate = 1.0;//s
};
