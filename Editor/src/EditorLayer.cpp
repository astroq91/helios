#include "EditorLayer.h"
#include <cstdint>
#include <filesystem>

#include <glm/ext/matrix_transform.hpp>
#include <imgui.h>

#include <Helios/Core/Log.h>
#include <Helios/Events/KeyEvents.h>
#include <Helios/Renderer/Renderer.h>
#include <optional>
#include <volk/volk.h>

#include "Helios/Core/Application.h"
#include "Helios/Core/IOUtils.h"
#include "Helios/Core/Math.h"
#include "Helios/ECSComponents/Components.h"
#include "Helios/Events/MouseEvents.h"
#include "Helios/Scene/Entity.h"
#include "Helios/Scene/Scene.h"
#include "Helios/Scene/SceneSerializer.h"
#include "ImGradient.h"
#include "ImGuizmo.h"
#include "glm/gtc/type_ptr.hpp"
#include "imgui_impl_vulkan.h"

using namespace Helios;
namespace fs = std::filesystem;

bool KeyPressed(KeyPressedEvent& e);

float g_speed = 100.0f;

constexpr Transform k_intial_scene_camera_transform = {
    .position = glm::vec3(0.0f, 5.0f, 3.0f),
};

struct CameraUniformBuffer {
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    alignas(16) glm::vec3 pos;
};

enum class WelcomeModalStep { Welcome, NewProject };
WelcomeModalStep g_current_welcome_modal_step = WelcomeModalStep::Welcome;
WelcomeModalStep g_previous_welcome_modal_step = WelcomeModalStep::Welcome;

EditorLayer::EditorLayer()
    : m_scene_camera({
          .viewport_width = 1,
          .viewport_height = 1,
          .initial_transform =
              {
                  .position = {0.0f, 3.0f, 4.0f},
              },
          .movement_speed = 6.0f,
          .sensitivity = 16.0f,
          .fov_y = 80,
      }) {
    m_fps_clock = std::chrono::high_resolution_clock::now();
}

EditorLayer::~EditorLayer() {
    if (m_scene) {
        delete m_scene;
    }
}

void EditorLayer::on_attach() {
    populate_viewport_data(m_editor_viewport);
    populate_viewport_data(m_game_viewport);
    setup_entity_picking();
    setup_editor_grid();

    new_scene({.reset_window_title = false});

    m_assets_browser.init();
    m_assets_browser.set_on_scene_selected_callback(
        [&](const auto& scene_path) { load_scene(scene_path); });
}

void EditorLayer::on_detach() {}

using namespace std::literals;

void EditorLayer::on_update(float ts) {
    if (!m_project) {
        return;
    }

    if (m_go_to_homescreen) {
        reset_editor();
        m_go_to_homescreen = false;
        return;
    }

    m_frame_counter++;
    auto time_now = std::chrono::high_resolution_clock::now();
    double duration =
        std::chrono::duration<double>(time_now - m_fps_clock) / 1s;
    if (duration > m_fps_sampling_rate) {
        m_fps_clock = std::chrono::high_resolution_clock::now();
        m_fps = m_frame_counter / duration;
        m_frame_counter = 0;
    }

    auto& app = Application::get();
    auto& renderer = app.get_renderer();
    auto& context = app.get_vulkan_manager()->get_context();

    if (viewport_resized(m_editor_viewport)) {
        resize_viewport(m_editor_viewport);
        m_scene_camera.on_resize(
            static_cast<uint32_t>(m_editor_viewport.size.x),
            static_cast<uint32_t>(m_editor_viewport.size.y));
        create_or_recreate_picking_images();
    }
    if (viewport_resized(m_game_viewport)) {
        resize_viewport(m_game_viewport);
    }

    if (m_editor_viewport.focused) {
        m_scene_camera.on_update(ts);
    }

    update_scene_camera_uniform();

    VulkanUtils::transition_image_layout(
        {.image =
             m_editor_viewport.images[app.get_current_frame()]->get_vk_image(),
         .old_layout = VK_IMAGE_LAYOUT_UNDEFINED,
         .new_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
         .src_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
         .src_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
         .dst_stage_mask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
         .command_buffer =
             renderer.get_current_command_buffer()->get_command_buffer()});
    VulkanUtils::transition_image_layout(
        {.image =
             m_game_viewport.images[app.get_current_frame()]->get_vk_image(),
         .old_layout = VK_IMAGE_LAYOUT_UNDEFINED,
         .new_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
         .src_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
         .src_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
         .dst_stage_mask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
         .command_buffer =
             renderer.get_current_command_buffer()->get_command_buffer()});

    m_scene->on_update(
        ts,
        {
            .color_image = m_editor_viewport.images[app.get_current_frame()],
            .color_image_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .color_clear_value = {0.25f, 0.25f, 0.25f, 1.0f},
            .depth_image =
                m_editor_viewport.depth_images[app.get_current_frame()],
            .width = static_cast<uint32_t>(m_editor_viewport.size.x),
            .height = static_cast<uint32_t>(m_editor_viewport.size.y),
        },
        {
            .color_image = m_game_viewport.images[app.get_current_frame()],
            .color_image_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .color_clear_value = {0.0f, 0.0f, 0.0f, 1.0f},
            .depth_image =
                m_game_viewport.depth_images[app.get_current_frame()],
            .width = static_cast<uint32_t>(m_game_viewport.size.x),
            .height = static_cast<uint32_t>(m_game_viewport.size.y),
        });

    update_entity_picking();
    render_editor_grid();

    // Transition the game viewport image (to be used in imgui image)
    VulkanUtils::transition_image_layout(
        {.image =
             m_game_viewport.images[app.get_current_frame()]->get_vk_image(),
         .old_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
         .new_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
         .src_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
         .dst_access_mask = VK_ACCESS_SHADER_READ_BIT,
         .src_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
         .dst_stage_mask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
         .command_buffer =
             renderer.get_current_command_buffer()->get_command_buffer()});

    // Now transition the editor viewport image (to be used in imgui image)
    VulkanUtils::transition_image_layout(
        {.image =
             m_editor_viewport.images[app.get_current_frame()]->get_vk_image(),
         .old_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
         .new_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
         .src_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
         .dst_access_mask = VK_ACCESS_SHADER_READ_BIT,
         .src_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
         .dst_stage_mask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
         .command_buffer =
             renderer.get_current_command_buffer()->get_command_buffer()});
}

void EditorLayer::on_fixed_update() {
    if (m_scene && m_scene->is_running()) {
        m_scene->on_fixed_update();
    }
}

void EditorLayer::on_event(Event& e) {
    EventDispatcher dispatcher(e);
    dispatcher.dispatch<KeyPressedEvent>([&](KeyPressedEvent& e) -> bool {
        if (e.get_key_code() == KeyCode::D1) {
            m_gizmo_operation = ImGuizmo::OPERATION::TRANSLATE;
        } else if (e.get_key_code() == KeyCode::D2) {
            m_gizmo_operation = ImGuizmo::OPERATION::ROTATE;
        } else if (e.get_key_code() == KeyCode::D3) {
            m_gizmo_operation = ImGuizmo::OPERATION::SCALE;
        } else if (e.get_key_code() == KeyCode::D4) {
            m_gizmo_operation = ImGuizmo::OPERATION::UNIVERSAL;
        } else if (e.get_key_code() == KeyCode::LeftControl) {
            m_use_snap = true;
        }

        return false;
    });

    dispatcher.dispatch<KeyReleasedEvent>([&](KeyReleasedEvent& e) {
        if (e.get_key_code() == KeyCode::LeftControl) {
            m_use_snap = false;
        }
        return false;
    });

    dispatcher.dispatch<MouseButtonPressedEvent>(
        [&](MouseButtonPressedEvent& e) -> bool {
            if (e.get_button() == MouseButton::Left &&
                m_editor_viewport.hovered &&
                m_editor_viewport.mouse_pos.x >= 0 &&
                m_editor_viewport.mouse_pos.x <= m_editor_viewport.size.x &&
                m_editor_viewport.mouse_pos.y >= 0 &&
                m_editor_viewport.mouse_pos.y <= m_editor_viewport.size.y) {
                m_use_mouse_picking = true;
            }

            return false;
        });

    m_scene_camera.on_event(e);
}

void EditorLayer::on_imgui_render() {
    ImGuizmo::BeginFrame();

    static bool xray = false;
    auto& app = Application::get();

    ImGuiIO& io = ImGui::GetIO();
    // Temporarily disable viewports, because this is causing issues (at
    // least on linux)
    io.ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable;

    static bool dockSpaceOpen = true;
    static bool opt_fullscreen = true;
    static bool opt_padding = false;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent
    // window not dockable into, because it would be confusing to have two
    // docking targets within each others.
    ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen) {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar |
                        ImGuiWindowFlags_NoCollapse |
                        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus |
                        ImGuiWindowFlags_NoNavFocus;
    } else {
        dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
    }

    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will
    // render our background and handle the pass-thru hole, so we ask
    // begin() to not render a background.
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    // Important: note that we proceed even if begin() returns false (aka
    // window is collapsed). This is because we want to keep our DockSpace()
    // active. If a DockSpace() is inactive, all active windows docked into
    // it will lose their parent and become undocked. We cannot preserve the
    // docking relationship between an active window and an inactive
    // docking, otherwise any change of dockspace/settings would lead to
    // windows being stuck in limbo and never being visible.
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace", &dockSpaceOpen, window_flags);
    {
        ImGui::PopStyleVar();

        if (opt_fullscreen)
            ImGui::PopStyleVar(2);

        // DockSpace
        ImGuiIO& io = ImGui::GetIO();
        ImGuiStyle& style = ImGui::GetStyle();
        float minWinSizeX = style.WindowMinSize.x;
        style.WindowMinSize.x = 370.0f;
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }

        style.WindowMinSize.x = minWinSizeX;

        if (!m_project && !m_show_new_project_window) {
            show_welcome_window();

        } else if (!m_project && m_show_new_project_window) {
            show_new_project_window();
        } else {
            if (ImGui::BeginMenuBar()) {

                if (ImGui::BeginMenu("Scene")) {
                    if (ImGui::MenuItem("New scene")) {
                        if (m_loaded_scene_path) {
                            save_scene(m_loaded_scene_path.value().string());
                        }
                        new_scene();
                    }
                    if (ImGui::MenuItem("Open scene...")) {
                        DialogReturn ret = IOUtils::open_file(
                            {
                                {
                                    .name = "Scene",
                                    .filter = "*.scene",
                                },
                            },
                            m_project.value().get_project_path());
                        load_scene(ret.path);
                    }
                    ImGui::BeginDisabled(!m_loaded_scene_path);
                    if (ImGui::MenuItem("Save scene")) {
                        stop_runtime();
                        save_scene(m_loaded_scene_path.value().string());
                    }
                    ImGui::EndDisabled();
                    if (ImGui::MenuItem("Save scene as...")) {
                        stop_runtime();
                        DialogReturn ret = IOUtils::save_file(
                            {
                                {
                                    .name = "Scene",
                                    .filter = "*.scene",
                                },
                            },
                            m_project.value().get_project_path());
                        if (!ret.path.empty()) {
                            m_loaded_scene_path = IOUtils::relative_path(
                                m_project.value().get_project_path(), ret.path);

                            save_scene(m_loaded_scene_path.value().string());
                            SceneSerializer scene_serializer(m_scene);
                            scene_serializer.deserialize_from_path(
                                m_loaded_scene_path.value().string());

                            update_window_title(
                                m_loaded_scene_path.value().string());
                        }
                    }
                    if (ImGui::MenuItem("Reload scene")) {
                        if (m_loaded_scene_path) {
                            Application::get()
                                .get_asset_manager()
                                .clear_assets();
                            load_scene(IOUtils::resolve_path(
                                m_project.value().get_project_path(),
                                m_loaded_scene_path.value()));
                        }
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Project")) {
                    if (ImGui::MenuItem("Set scene as default")) {
                        if (m_loaded_scene_path) {
                            m_project.value().set_default_scene(
                                m_loaded_scene_path.value().string());
                        } else {
                            HL_ERROR("Tried to set default scene path, but no "
                                     "loaded scene");
                        }
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Editor")) {
                    if (ImGui::MenuItem("Go to homescreen")) {
                        m_go_to_homescreen = true;
                    }
                    ImGui::EndMenu();
                }

                float button1Width = ImGui::CalcTextSize("Run").x +
                                     ImGui::GetStyle().FramePadding.x * 2;
                float button2Width = ImGui::CalcTextSize("Stop").x +
                                     ImGui::GetStyle().FramePadding.x * 2;
                float button3Width = ImGui::CalcTextSize("Copy runtime").x +
                                     ImGui::GetStyle().FramePadding.x * 2;

                // Total width of all buttons plus spacing between them
                float totalButtonWidth =
                    button1Width + button2Width + button3Width;
                float spacing =
                    ImGui::GetStyle()
                        .ItemSpacing.x; // Default spacing between items
                float totalWidthWithSpacing =
                    totalButtonWidth +
                    2 * spacing; // Two gaps for three buttons

                // Calculate the offset to center all three buttons
                float menuBarWidth = ImGui::GetWindowWidth();
                float offsetX = (menuBarWidth - totalWidthWithSpacing) / 2.0f;

                // Set cursor position to start drawing buttons
                ImGui::SameLine(offsetX);

                ImGui::BeginDisabled(m_scene->is_running());
                if (ImGui::Button("Run")) {
                    SceneSerializer scene_serializer(m_scene);
                    scene_serializer.serialize_to_string(m_scene_copy);
                    m_scene->start_runtime();
                }
                ImGui::EndDisabled();

                ImGui::BeginDisabled(!m_scene->is_running());
                if (ImGui::Button("Stop")) {
                    stop_runtime();
                }
                ImGui::EndDisabled();

                ImGui::BeginDisabled(!m_scene->is_running());
                if (ImGui::Button("Copy runtime")) {
                    SceneSerializer scene_serializer(m_scene);
                    scene_serializer.serialize_to_string(m_scene_copy);
                }
                ImGui::EndDisabled();

                ImGui::EndMenuBar();
            }

            // Viewport //
            ImGui::SetNextWindowSizeConstraints(ImVec2(100, 100),
                                                ImVec2(10000, 10000));
            ImGui::Begin("Game");
            {

                m_game_viewport.size = ImGui::GetContentRegionAvail();

                ImVec2 image_pos = ImGui::GetCursorScreenPos();

                ImGui::Image(m_game_viewport.handles[app.get_current_frame()],
                             m_game_viewport.size);

                if (m_scene) {
                    m_scene->set_game_viewport_focused(
                        ImGui::IsWindowFocused());
                    m_scene->set_game_viewport_position(
                        {image_pos.x, image_pos.y});
                }
            }
            ImGui::End();

            ImGui::SetNextWindowSizeConstraints(ImVec2(100, 100),
                                                ImVec2(10000, 10000));
            ImGui::Begin("Editor");
            {
                m_editor_viewport.size = ImGui::GetContentRegionAvail();
                ImVec2 global_mouse_pos = ImGui::GetIO().MousePos;

                // get the current window's position
                ImVec2 window_pos = ImGui::GetWindowPos();

                // Calculate the mouse position relative to the window's
                // top-left corner
                ImVec2 mouse_pos_in_window =
                    ImVec2(global_mouse_pos.x - window_pos.x,
                           global_mouse_pos.y - window_pos.y);

                // Optionally, you can also consider the window's title bar
                // height and any window padding as follows
                float title_bar_height = ImGui::GetStyle().FramePadding.y +
                                         ImGui::GetFontSize() +
                                         ImGui::GetStyle().WindowPadding.y;
                m_editor_viewport.mouse_pos = ImVec2(
                    mouse_pos_in_window.x - ImGui::GetStyle().WindowPadding.x,
                    mouse_pos_in_window.y - title_bar_height);

                ImVec2 window_pos_without_padding =
                    ImVec2(window_pos.x - ImGui::GetStyle().WindowPadding.x,
                           window_pos.y - title_bar_height);

                m_editor_viewport.focused = ImGui::IsWindowFocused();
                m_editor_viewport.hovered = ImGui::IsWindowHovered();
                Application::get().get_imgui_layer()->set_block_events(
                    !m_editor_viewport.focused);

                ImGui::Image(m_editor_viewport.handles[app.get_current_frame()],
                             m_editor_viewport.size);

                // Disable manipulation if an entity has a dynamic rb,
                // without override_dynamic_physics=true
                bool disable_gizmo = false;
                if (m_selected_entity != k_no_entity) {
                    RigidBodyComponent* rb =
                        m_selected_entity
                            .try_get_component<RigidBodyComponent>();

                    if (m_scene->is_running() && rb &&
                        rb->type == RigidBodyType::Dynamic && !rb->kinematic &&
                        !rb->override_dynamic_physics) {
                        disable_gizmo = true;
                    }
                }

                if (m_selected_entity != k_no_entity &&
                    m_selected_entity_transform && !disable_gizmo) {
                    ImGuizmo::BeginFrame();
                    ImGuizmo::SetOrthographic(false);
                    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());

                    // x += 19, y += 59 are magic numbers for dealing with
                    // the viewport image being offset from the imgui window
                    // (ie. padding)
                    ImGuizmo::SetRect(window_pos_without_padding.x + 19,
                                      window_pos_without_padding.y + 59,
                                      m_editor_viewport.size.x,
                                      m_editor_viewport.size.y);

                    glm::mat4 model =
                        m_selected_entity_transform->to_transform().ToMat4();

                    PerspectiveCamera cam = m_scene_camera.get_camera();
                    cam.projection_matrix[1][1] *= -1;

                    glm::vec3 snap = {0.5f, 0.5f, 0.5f};

                    ImGuizmo::Manipulate(glm::value_ptr(cam.view_matrix),
                                         glm::value_ptr(cam.projection_matrix),
                                         m_gizmo_operation, m_gizmo_mode,
                                         glm::value_ptr(model), nullptr,
                                         m_use_snap ? &snap.x : nullptr);

                    if (ImGuizmo::IsUsing()) {
                        Math::decompose_transform(
                            model, m_selected_entity_transform->position,
                            m_selected_entity_transform->rotation,
                            m_selected_entity_transform->scale);
                        m_scene->on_entity_transform_updated(m_selected_entity);

                        m_use_mouse_picking = false;
                    }

                    if (ImGuizmo::IsOver()) {
                        m_using_gizmo = true;
                    } else {
                        m_using_gizmo = false;
                    }
                }
            }
            ImGui::End();

            ImGui::Begin("Entity Browser");
            {
                if (ImGui::Button("Create Entity")) {
                    auto entity = m_scene->create_entity("New Entity");
                    select_entity(entity);
                }
                ImGui::Separator();

                m_scene->sort_component<NameComponent>();
                auto view = m_scene->get_view<NameComponent>(
                    entt::exclude<ParentComponent>);

                for (const auto& [entity, name] : view.each()) {
                    draw_entity_list_entry(static_cast<uint32_t>(entity), name);
                }
            }
            ImGui::End();

            ImGui::Begin("Settings");
            {
                static bool toggle = false;
                if (ImGui::Checkbox("World gizmo mode", &toggle)) {
                    if (toggle) {
                        m_gizmo_mode = ImGuizmo::MODE::WORLD;
                    } else {
                        m_gizmo_mode = ImGuizmo::MODE::LOCAL;
                    }
                }

                static int min_instances_for_mt =
                    app.get_renderer().get_min_instances_for_mt();
                static int num_threads_for_mt =
                    app.get_renderer().get_num_threads_for_instancing();

                if (ImGui::TreeNode("Instancing")) {
                    if (ImGui::TreeNode("Multithreading")) {
                        if (ImGui::InputInt("Minimum number of instances",
                                            &min_instances_for_mt)) {
                            app.get_renderer().set_min_instances_for_mt(
                                min_instances_for_mt);
                        }

                        if (ImGui::InputInt("Number of threads",
                                            &num_threads_for_mt)) {
                            app.get_renderer().set_num_threads_for_instancing(
                                num_threads_for_mt);
                        }
                        ImGui::TreePop();
                    }

                    ImGui::TreePop();
                }

                static bool vsync = app.get_renderer().vsync_enabled();

                if (ImGui::Checkbox("Vertical Sync", &vsync)) {
                    app.get_renderer().recreate_swapchain_next_frame(vsync);
                }
            }
            ImGui::End();

            ImGui::Begin("Statistics");
            {
                ImGui::Text("FPS: %d", m_fps);
                ImGui::InputDouble("FPS sampling rate", &m_fps_sampling_rate);
            }
            ImGui::End();
            m_components_browser.on_update(
                m_scene, m_scene->get_entity(m_selected_entity),
                m_project.value());
            m_assets_browser.on_update();
        }
    }
    ImGui::End();
}

void EditorLayer::setup_editor_grid() {
    SharedPtr<Shader> vertex_shader = Shader::create(
        "editor_grid.vert", RESOURCES_PATH "shaders/bin/EditorGrid.vert");
    SharedPtr<Shader> fragment_shader = Shader::create(
        "editor_grid.frag", RESOURCES_PATH "shaders/bin/EditorGrid.frag");

    m_grid_pipeline = Pipeline::create_unique({
        .color_attachment_format =
            Application::get().get_renderer().get_swapchain()->get_vk_format(),
        .descriptor_set_layouts = {m_scene_camera_set_layout},
        .vertex_shader = vertex_shader,
        .fragment_shader = fragment_shader,
        .vertex_buffer_descriptions = {},
        .color_blend_attachments =
            {
                VkPipelineColorBlendAttachmentState{

                    .blendEnable = VK_TRUE, // Enable blending
                    .srcColorBlendFactor =
                        VK_BLEND_FACTOR_SRC_ALPHA, // Source blend factor
                    .dstColorBlendFactor =
                        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, // Destination
                                                             // blend factor
                    .colorBlendOp = VK_BLEND_OP_ADD,         // Blend operation
                                                     // (equivalent to addition)
                    .srcAlphaBlendFactor =
                        VK_BLEND_FACTOR_SRC_ALPHA, // Alpha source blend
                                                   // factor
                    .dstAlphaBlendFactor =
                        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, // Alpha
                                                             // destination
                                                             // blend factor
                    .alphaBlendOp = VK_BLEND_OP_ADD, // Alpha blend operation
                    .colorWriteMask =
                        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT |
                        VK_COLOR_COMPONENT_A_BIT, // Write all RGBA
                                                  // components
                },
            },
    });
}

void EditorLayer::populate_viewport_data(ViewportData& viewport) {
    auto& app = Application::get();
    auto& renderer = app.get_renderer();
    const auto& vulkan_context =
        Application::get().get_vulkan_manager()->get_context();

    // Set the default viewport size to non-zero in case it is not updated
    // before rendering
    viewport.size = {1, 1};

    viewport.sampler = TextureSampler::create_unique();

    // For frames in flight
    viewport.images.resize(app.get_max_frames_in_flight());
    viewport.depth_images.resize(app.get_max_frames_in_flight());
    viewport.handles.resize(app.get_max_frames_in_flight());
    for (uint32_t i = 0; i < app.get_max_frames_in_flight(); i++) {
        viewport.images[i] = Image::create(ImageSpec{
            .width = renderer.get_swapchain()->get_vk_extent().width,
            .height = renderer.get_swapchain()->get_vk_extent().height,
            .format = renderer.get_swapchain()->get_vk_format(),
            .aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT,
            .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                     VK_IMAGE_USAGE_SAMPLED_BIT,
            .memory_property = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT});

        viewport.handles[i] = ImGui_ImplVulkan_AddTexture(
            viewport.sampler->get_vk_sampler(),
            viewport.images[i]->get_vk_image_view(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        viewport.depth_images[i] = Image::create(ImageSpec{
            .width = renderer.get_swapchain()->get_vk_extent().width,
            .height = renderer.get_swapchain()->get_vk_extent().height,
            .format = VulkanUtils::find_depth_format(Application::get()
                                                         .get_vulkan_manager()
                                                         ->get_context()
                                                         .physical_device),
            .aspect_flags = VK_IMAGE_ASPECT_DEPTH_BIT,
            .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                     VK_IMAGE_USAGE_SAMPLED_BIT,
            .memory_property = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT});

        VkCommandBuffer command_buffer =
            VulkanUtils::begin_single_time_commands(
                vulkan_context.device, vulkan_context.command_pool);

        // Transition the depth image
        VulkanUtils::transition_image_layout(
            {.image = viewport.depth_images[i]->get_vk_image(),
             .old_layout = VK_IMAGE_LAYOUT_UNDEFINED,
             .new_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
             .src_access_mask = 0,
             .dst_access_mask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
             .src_stage_mask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
             .dst_stage_mask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
             .command_buffer = command_buffer});

        VulkanUtils::end_single_time_commands(
            command_buffer, vulkan_context.device, vulkan_context.command_pool,
            vulkan_context.graphics_queue);
    }
}

void EditorLayer::render_editor_grid() {
    auto& app = Application::get();
    auto& renderer = app.get_renderer();

    renderer.begin_rendering({
        .color_image = m_editor_viewport.images[app.get_current_frame()],
        .color_image_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .color_load_op = VK_ATTACHMENT_LOAD_OP_LOAD,
        .color_store_op = VK_ATTACHMENT_STORE_OP_STORE,
        .depth_image = m_editor_viewport.depth_images[app.get_current_frame()],
        .depth_load_op = VK_ATTACHMENT_LOAD_OP_LOAD,
        .depth_store_op = VK_ATTACHMENT_STORE_OP_STORE,
        .width = static_cast<uint32_t>(m_editor_viewport.size.x),
        .height = static_cast<uint32_t>(m_editor_viewport.size.y),
    });
    {
        vkCmdBindPipeline(
            renderer.get_current_command_buffer()->get_command_buffer(),
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_grid_pipeline->get_vk_pipeline());

        vkCmdBindDescriptorSets(
            renderer.get_current_command_buffer()->get_command_buffer(),
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_entity_picking_pipeline->get_vk_layout(), 0, 1,
            &m_scene_camera_descriptor_sets[app.get_current_frame()]
                 ->get_vk_set(),
            0, nullptr);

        vkCmdDraw(renderer.get_current_command_buffer()->get_command_buffer(),
                  6, 1, 0, 0);
    }
    renderer.end_rendering();
}

void EditorLayer::update_entity_picking() {
    auto& app = Application::get();
    auto& renderer = app.get_renderer();
    if (m_use_mouse_picking && !m_using_gizmo) {
        auto view =
            m_scene->get_view<TransformComponent, MeshRendererComponent>();

        if (view.front() != entt::null) {
            // First we prepare the data
            for (auto [entity, transform_component, mesh_component] :
                 view.each()) {
                m_entity_picking_shader_data.push_back(
                    {.model = transform_component.to_transform().ToMat4(),
                     .entity_id = static_cast<uint32_t>(entity)});
            }

            // Then copy the data
            m_entity_picking_buffers[app.get_current_frame()]->insert_memory(
                m_entity_picking_shader_data.data(),
                sizeof(EntityPickingShaderData) *
                    m_entity_picking_shader_data.size());

            // Transition the entity picking image
            VulkanUtils::transition_image_layout(
                {.image = m_entity_picking_images[app.get_current_frame()]
                              ->get_vk_image(),
                 .old_layout = VK_IMAGE_LAYOUT_UNDEFINED,
                 .new_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                 .dst_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                 .src_stage_mask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                 .dst_stage_mask =
                     VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                 .command_buffer = renderer.get_current_command_buffer()
                                       ->get_command_buffer()});

            // begin rendering
            renderer.begin_rendering(
                {.color_image =
                     m_entity_picking_images[app.get_current_frame()],
                 .color_image_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                 .color_load_op = VK_ATTACHMENT_LOAD_OP_CLEAR,
                 .color_store_op = VK_ATTACHMENT_STORE_OP_STORE,
                 .color_clear_value = {1.0f, 1.0f, 1.0f, 1.0f},
                 .depth_image =
                     m_entity_picking_depth_images[app.get_current_frame()],
                 .depth_load_op = VK_ATTACHMENT_LOAD_OP_CLEAR,
                 .depth_store_op = VK_ATTACHMENT_STORE_OP_STORE,
                 .width = static_cast<uint32_t>(m_editor_viewport.size.x),
                 .height = static_cast<uint32_t>(m_editor_viewport.size.y)});

            vkCmdBindPipeline(
                renderer.get_current_command_buffer()->get_command_buffer(),
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_entity_picking_pipeline->get_vk_pipeline());

            // Then we record everything
            uint32_t offset = 0;
            for (auto [entity, transform_component, mesh_component] :
                 view.each()) {
                VkBuffer buffers[2] = {
                    mesh_component.mesh->get_vertex_buffer()->get_vk_buffer(),
                    m_entity_picking_buffers[app.get_current_frame()]
                        ->get_vk_buffer()};
                VkDeviceSize offsets[2] = {0, offset};
                vkCmdBindVertexBuffers(
                    renderer.get_current_command_buffer()->get_command_buffer(),
                    0, 2, buffers, offsets);

                vkCmdBindIndexBuffer(
                    renderer.get_current_command_buffer()->get_command_buffer(),
                    mesh_component.mesh->get_index_buffer()->get_vk_buffer(), 0,
                    VK_INDEX_TYPE_UINT32);

                VkDescriptorSet sets[] = {
                    m_scene_camera_descriptor_sets[app.get_current_frame()]
                        ->get_vk_set(),
                };

                vkCmdBindDescriptorSets(
                    renderer.get_current_command_buffer()->get_command_buffer(),
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    m_entity_picking_pipeline->get_vk_layout(), 0, 1, sets, 0,
                    nullptr);

                vkCmdDrawIndexed(
                    renderer.get_current_command_buffer()->get_command_buffer(),
                    mesh_component.mesh->get_index_buffer()->get_index_count(),
                    1, 0, 0, 0);

                offset += sizeof(EntityPickingShaderData);
            }

            renderer.end_rendering();
            renderer.submit_command_buffer();

            VulkanUtils::transition_image_layout({
                .image = m_entity_picking_images[app.get_current_frame()]
                             ->get_vk_image(),
                .old_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .new_layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                .src_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                .dst_access_mask = VK_ACCESS_TRANSFER_READ_BIT,
                .src_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                .dst_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT,
                .command_buffer =
                    renderer.get_current_command_buffer()->get_command_buffer(),
            });

            SharedPtr<Buffer> staging_buffer = Buffer::create(
                m_entity_picking_images[app.get_current_frame()]->get_vk_size(),
                VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                true);

            VulkanUtils::copy_image_to_buffer(
                renderer.get_current_command_buffer()->get_command_buffer(),
                m_entity_picking_images[app.get_current_frame()]
                    ->get_vk_image(),
                m_entity_picking_images[app.get_current_frame()]->get_width(),
                m_entity_picking_images[app.get_current_frame()]->get_height(),
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, // TODO: Transition to
                // VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL?
                staging_buffer->get_vk_buffer());

            renderer.submit_command_buffer();

            uint32_t pixel_offset =
                (static_cast<uint32_t>(m_editor_viewport.mouse_pos.y) *
                     m_entity_picking_images[app.get_current_frame()]
                         ->get_width() +
                 static_cast<uint32_t>(m_editor_viewport.mouse_pos.x)) *
                4;

            uint8_t* pixel =
                static_cast<uint8_t*>(staging_buffer->get_mapped_memory()) +
                pixel_offset;

            // No entity selected
            if (static_cast<int>(pixel[0]) == 255 &&
                static_cast<int>(pixel[1]) == 255 &&
                static_cast<int>(pixel[2]) == 255) {
                m_selected_entity = Entity();
            } else {
                select_entity(static_cast<int>(pixel[0]) |
                              (static_cast<int>(pixel[1]) << 8) |
                              (static_cast<int>(pixel[2]) << 16));
            }
            m_entity_picking_shader_data.clear();
        }
        m_use_mouse_picking = false;
    }
}

bool EditorLayer::viewport_resized(const ViewportData& viewport) {
    const auto& app = Application::get();
    // Check if the viewport has been updated (from ImGui)
    if (viewport.size.x > 0 && viewport.size.y > 0 &&
        (static_cast<uint32_t>(viewport.size.x) !=
             viewport.images[app.get_current_frame()]->get_width() ||
         static_cast<uint32_t>(viewport.size.y) !=
             viewport.images[app.get_current_frame()]->get_height())) {
        return true;
    }
    return false;
}

void EditorLayer::resize_viewport(ViewportData& viewport) {
    auto& app = Application::get();
    auto& renderer = app.get_renderer();

    viewport.images[app.get_current_frame()] = Image::create(
        ImageSpec{.width = static_cast<uint32_t>(viewport.size.x),
                  .height = static_cast<uint32_t>(viewport.size.y),
                  .format = renderer.get_swapchain()->get_vk_format(),
                  .aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT,
                  .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                           VK_IMAGE_USAGE_SAMPLED_BIT,
                  .memory_property = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT});

    // Transition to SHADER_READ because if resize_viewport is called from
    // the imgui render loop, then the viewport won't be transitioned from
    // UNDEFINED before being rendered to the viewport texture
    VulkanUtils::transition_image_layout(
        {.image = viewport.images[app.get_current_frame()]->get_vk_image(),
         .old_layout = VK_IMAGE_LAYOUT_UNDEFINED,
         .new_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
         .src_access_mask = 0,
         .dst_access_mask = VK_ACCESS_SHADER_READ_BIT,
         .src_stage_mask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
         .dst_stage_mask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
         .command_buffer =
             renderer.get_current_command_buffer()->get_command_buffer()});

    viewport.depth_images[app.get_current_frame()] = Image::create(ImageSpec{
        .width = static_cast<uint32_t>(viewport.size.x),
        .height = static_cast<uint32_t>(viewport.size.y),
        .format = VulkanUtils::find_depth_format(Application::get()
                                                     .get_vulkan_manager()
                                                     ->get_context()
                                                     .physical_device),
        .aspect_flags = VK_IMAGE_ASPECT_DEPTH_BIT,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                 VK_IMAGE_USAGE_SAMPLED_BIT,
        .memory_property = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT});

    // Transition the depth image
    VulkanUtils::transition_image_layout(
        {.image =
             viewport.depth_images[app.get_current_frame()]->get_vk_image(),
         .old_layout = VK_IMAGE_LAYOUT_UNDEFINED,
         .new_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
         .src_access_mask = 0,
         .dst_access_mask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
         .src_stage_mask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
         .dst_stage_mask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
         .command_buffer =
             renderer.get_current_command_buffer()->get_command_buffer()});

    ImGui_ImplVulkan_RemoveTexture(viewport.handles[app.get_current_frame()]);

    viewport.handles[app.get_current_frame()] = ImGui_ImplVulkan_AddTexture(
        viewport.sampler->get_vk_sampler(),
        viewport.images[app.get_current_frame()]->get_vk_image_view(),
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void EditorLayer::update_scene_camera_uniform() {
    // Now set up the camera uniform
    PerspectiveCamera camera = m_scene_camera.get_camera();

    CameraUniformBuffer ubo{
        .view = camera.view_matrix,
        .proj = camera.projection_matrix,
        .pos = camera.position,
    };

    memcpy(
        m_scene_camera_uniform_buffers[Application::get().get_current_frame()]
            ->get_mapped_memory(),
        &ubo, sizeof(ubo));
}

void EditorLayer::create_or_recreate_picking_images() {
    auto& renderer = Application::get().get_renderer();
    const auto& vulkan_context =
        Application::get().get_vulkan_manager()->get_context();
    const uint32_t max_frames_in_flight =
        Application::get().get_max_frames_in_flight();

    m_entity_picking_images.resize(max_frames_in_flight);
    m_entity_picking_depth_images.resize(max_frames_in_flight);

    for (size_t i = 0; i < max_frames_in_flight; i++) {
        m_entity_picking_images[i] = Image::create(ImageSpec{
            .width = renderer.get_swapchain()->get_vk_extent().width,
            .height = renderer.get_swapchain()->get_vk_extent().height,
            .format = VK_FORMAT_R8G8B8A8_UNORM,
            .aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT,
            .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                     VK_IMAGE_USAGE_SAMPLED_BIT |
                     VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
            .memory_property = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT});

        m_entity_picking_depth_images[i] = Image::create(ImageSpec{
            .width = renderer.get_swapchain()->get_vk_extent().width,
            .height = renderer.get_swapchain()->get_vk_extent().height,
            .format = VulkanUtils::find_depth_format(Application::get()
                                                         .get_vulkan_manager()
                                                         ->get_context()
                                                         .physical_device),
            .aspect_flags = VK_IMAGE_ASPECT_DEPTH_BIT,
            .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                     VK_IMAGE_USAGE_SAMPLED_BIT,
            .memory_property = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT});

        VkCommandBuffer command_buffer =
            VulkanUtils::begin_single_time_commands(
                vulkan_context.device, vulkan_context.command_pool);

        // Transition the depth image
        VulkanUtils::transition_image_layout(
            {.image = m_entity_picking_depth_images[i]->get_vk_image(),
             .old_layout = VK_IMAGE_LAYOUT_UNDEFINED,
             .new_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
             .src_access_mask = 0,
             .dst_access_mask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
             .src_stage_mask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
             .dst_stage_mask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
             .command_buffer = command_buffer});

        VulkanUtils::end_single_time_commands(
            command_buffer, vulkan_context.device, vulkan_context.command_pool,
            vulkan_context.graphics_queue);
    }
}

void EditorLayer::setup_entity_picking() {
    auto& renderer = Application::get().get_renderer();
    const auto& vulkan_context =
        Application::get().get_vulkan_manager()->get_context();
    const uint32_t max_frames_in_flight =
        Application::get().get_max_frames_in_flight();

    VertexBufferDescription desc{VertexInputRate::Instance,
                                 1,
                                 {
                                     {VertexAttributeFormat::FLOAT4, 3},
                                     {VertexAttributeFormat::FLOAT4, 4},
                                     {VertexAttributeFormat::FLOAT4, 5},
                                     {VertexAttributeFormat::FLOAT4, 6},
                                     {VertexAttributeFormat::UINT32, 7},
                                 }};

    auto vertex_shader =
        Shader::create("", RESOURCES_PATH "shaders/bin/EntityPicking.vert");
    auto fragment_shader =
        Shader::create("", RESOURCES_PATH "shaders/bin/EntityPicking.frag");

    m_scene_camera_set_layout =
        DescriptorSetLayout::create({DescriptorSetLayoutBinding{
            .binding = 0,
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .descriptor_count = 1,
        }});

    m_scene_camera_set_pool = DescriptorPool::create(
        max_frames_in_flight * 2,
        {
            VkDescriptorPoolSize{
                .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = max_frames_in_flight,
            },
            VkDescriptorPoolSize{.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                 .descriptorCount = max_frames_in_flight},
        });

    create_or_recreate_picking_images();

    m_entity_picking_buffers.resize(max_frames_in_flight);
    m_scene_camera_uniform_buffers.resize(max_frames_in_flight);
    m_scene_camera_descriptor_sets.resize(max_frames_in_flight);

    for (size_t i = 0; i < max_frames_in_flight; i++) {
        m_entity_picking_buffers[i] = VertexBuffer::create(
            nullptr, sizeof(EntityPickingShaderData) * k_max_meshes);

        m_scene_camera_uniform_buffers[i] =
            Buffer::create(sizeof(CameraUniformBuffer),
                           VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
                               VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                           true);
        m_scene_camera_descriptor_sets[i] = DescriptorSet::create_unique(
            m_scene_camera_set_pool, m_scene_camera_set_layout,
            {DescriptorSpec{
                .binding = 0,
                .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptor_class = DescriptorClass::Buffer,
                .buffer = m_scene_camera_uniform_buffers[i],
            }});
    }

    m_entity_picking_pipeline = Pipeline::create_unique(
        {VK_FORMAT_R8G8B8A8_UNORM,
         {m_scene_camera_set_layout},
         vertex_shader,
         fragment_shader,
         {desc, renderer.get_meshes_vertices_description()}});
}

void EditorLayer::select_entity(uint32_t entity) {
    m_selected_entity = m_scene->get_entity(entity);
    m_selected_entity_transform =
        m_scene->get_entity(entity).try_get_component<TransformComponent>();
}

void EditorLayer::new_scene(const NewSceneInfo& info) {
    if (m_scene) {
        delete m_scene;
    }
    m_scene = new Scene(&m_scene_camera);

    if (info.reset_scene_camera) {
        m_scene_camera.reset_camera();
    }
    if (info.reset_selected_entity) {
        m_selected_entity = Entity();
    }
    if (info.reset_scene_path) {
        m_loaded_scene_path = std::nullopt;
    }
    if (info.reset_window_title) {
        update_window_title(std::nullopt);
    }
}

void EditorLayer::save_scene(const std::string& path) {
    SceneSerializer scene_serializer(m_scene);
    scene_serializer.serialize_to_path(path);
}

void EditorLayer::show_welcome_window() {
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 center = ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    ImGui::Begin("Welcome", nullptr,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoMove);

    ImGui::SetWindowSize({225, 100});

    ImGui::Text("Select one of the following:");
    ImGui::Spacing();

    if (ImGui::Button("New Project", ImVec2(200, 0))) {
        m_show_new_project_window = true;
    }

    if (ImGui::Button("Open Project", ImVec2(200, 0))) {
        DialogReturn ret = IOUtils::select_folder();
        if (!ret.path.empty()) {
            fs::path project_path = fs::path(ret.path);
            project_path += fs::path::preferred_separator;
            if (!new_project(project_path)) {
                HL_ERROR("Could not load project");
            }
        }
    }

    ImGui::End();
}

void EditorLayer::show_new_project_window() {
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 center = ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    ImGui::Begin("New Project", nullptr,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoMove);

    ImGui::SetWindowSize({300, 150});

    ImGui::Text("Create a New Project");
    ImGui::Spacing();

    static std::string project_name;
    static std::filesystem::path project_folder;

    // Input field for project name
    char name_buffer[128];
    strncpy(name_buffer, project_name.c_str(), sizeof(name_buffer));
    if (ImGui::InputText("Project Name", name_buffer, sizeof(name_buffer))) {
        project_name = name_buffer;
    }

    // Select folder button
    if (ImGui::Button("Select Folder", ImVec2(200, 0))) {
        DialogReturn ret = IOUtils::select_folder();
        if (!ret.path.empty()) {
            project_folder = ret.path;
        }
    }

    if (ImGui::Button("Back")) {
        m_show_new_project_window = false;
    }
    ImGui::SameLine();

    // Create project button
    if (ImGui::Button("Create Project", ImVec2(156, 0))) {
        if (!project_name.empty() && !project_folder.empty()) {
            try {
                fs::path project_path = project_folder / project_name;
                if (!fs::exists(project_path)) {
                    if (new_project(project_path)) {
                        m_show_new_project_window = false;
                    } else {
                        HL_ERROR("Could not load project");
                    }
                } else {
                    HL_ERROR("Directory {} already exists",
                             project_path.string());
                }
            } catch (const fs::filesystem_error& e) {
                HL_ERROR("Filesystem error {}", e.what());
            }
        }
    }

    ImGui::End();
}

void EditorLayer::update_window_title(
    const std::optional<std::string>& scene_path) {
    if (m_project) {
        Application::get().get_window().set_title(
            "Helios - " + m_project.value().get_properties().name + " [" +
            scene_path.value_or("Untitled scene") + "]");
    } else {
        Application::get().get_window().set_title("Helios");
    }
}

void EditorLayer::reset_editor() {
    if (m_scene) {
        delete m_scene;
        m_scene = nullptr;
    }
    m_project = std::nullopt;
    m_loaded_scene_path = std::nullopt;
    m_selected_entity = Entity();
    g_current_welcome_modal_step = WelcomeModalStep::Welcome;
    g_previous_welcome_modal_step = WelcomeModalStep::Welcome;
    update_window_title(std::nullopt);
    Application::get().get_asset_manager().clear_assets();
}

void EditorLayer::stop_runtime() {
    if (!m_scene->is_running()) {
        return;
    }

    new_scene({
        .reset_selected_entity = false,
        .reset_scene_camera = false,
        .reset_scene_path = false,
        .reset_window_title = false,
    });

    SceneSerializer scene_serializer(m_scene);
    scene_serializer.deserialize_from_string(m_scene_copy);

    if (m_loaded_scene_path.has_value()) {
        update_window_title(m_loaded_scene_path.value().string());
    }

    // The previous scene pointer is now invalid
    m_selected_entity = m_scene->get_entity(m_selected_entity);
    if (m_selected_entity != k_no_entity) {
        m_selected_entity_transform =
            m_selected_entity.try_get_component<TransformComponent>();
    } else {
        m_selected_entity_transform = nullptr;
    }
}

void EditorLayer::load_scene(const std::filesystem::path& path) {
    if (!path.empty()) {
        new_scene({.reset_window_title = false});

        m_loaded_scene_path =
            IOUtils::relative_path(m_project.value().get_project_path(), path);

        SceneSerializer scene_serializer(m_scene);
        scene_serializer.deserialize_from_path(
            m_loaded_scene_path.value().string());

        update_window_title(m_loaded_scene_path.value().string());
    }
}

void EditorLayer::draw_entity_list_entry(uint32_t entity,
                                         const NameComponent& name) {
    ImGui::PushID(static_cast<int>(entity));

    bool is_selected = (m_selected_entity == entity);

    const std::vector<uint32_t>* children =
        m_scene->try_get_entity_children(m_scene->get_entity(entity));
    if (children && !children->empty()) {
        if (ImGui::TreeNodeEx(
                name.name.c_str(),
                ImGuiTreeNodeFlags_OpenOnArrow |
                    ImGuiTreeNodeFlags_SpanFullWidth |
                    (is_selected ? ImGuiTreeNodeFlags_Selected : 0))) {
            drag_drop_entity_list_entry(entity, name.name);
            if (ImGui::IsItemClicked()) {
                select_entity(entity);
            }
            for (uint32_t child : *children) {
                draw_entity_list_entry(
                    child,
                    m_scene->get_entity(child).get_component<NameComponent>());
            }
            ImGui::TreePop();
        } else {
            drag_drop_entity_list_entry(entity, name.name);
            if (ImGui::IsItemClicked()) {
                select_entity(entity);
            }
        }

    } else {
        if (ImGui::Selectable(name.name.c_str(), is_selected)) {
            select_entity(entity);
        }
        drag_drop_entity_list_entry(entity, name.name);
    }

    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        ImGui::OpenPopup("Remove");
    }

    if (ImGui::BeginPopup("Remove")) {
        if (ImGui::MenuItem("Remove Entity")) {
            m_scene->destroy_entity(m_selected_entity);
        }

        ImGui::EndPopup();
    }

    ImGui::PopID();
}

void EditorLayer::drag_drop_entity_list_entry(uint32_t entity,
                                              const std::string& name) {
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
        ImGui::SetDragDropPayload("ENTITY_BROWSER_ENTITY", &entity,
                                  sizeof(uint32_t));
        ImGui::Text("%s", name.c_str());
        ImGui::EndDragDropSource();
    }

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload =
                ImGui::AcceptDragDropPayload("ENTITY_BROWSER_ENTITY")) {
            uint32_t received_entity_id =
                *static_cast<uint32_t*>(payload->Data);

            if (received_entity_id != entity) {
                Entity received_entity =
                    m_scene->get_entity(received_entity_id);
                if (received_entity.has_component<ParentComponent>()) {
                    received_entity.remove_component<ParentComponent>();
                }
                auto& parent_component =
                    received_entity.add_component<ParentComponent>(entity);

                auto received_entity_transform =
                    received_entity.try_get_component<TransformComponent>();
                if (received_entity_transform) {
                    m_scene->on_entity_transform_updated(received_entity);
                }
            }
        }
        ImGui::EndDragDropTarget();
    }
}

bool EditorLayer::new_project(const std::filesystem::path& project_path) {
    m_project = Project(project_path);
    if (m_project.value().is_valid()) {
        Application::get().set_asset_base_path(
            m_project.value().get_project_path());
        m_assets_browser.set_project(&m_project.value());

        if (m_project.value().get_properties().default_scene) {
            new_scene({.reset_window_title = false});
            m_loaded_scene_path = fs::path(
                m_project.value().get_properties().default_scene.value());

            SceneSerializer scene_serializer(m_scene);
            scene_serializer.deserialize_from_path(
                m_loaded_scene_path.value().string());
        }
        update_window_title(m_project.value().get_properties().default_scene);

        Application::get().set_fixed_update_rate(
            m_project->get_properties().fixed_update_rate);

        return true;
    }
    return false;
}
