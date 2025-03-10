#include "SandboxLayer.h"

#include <Helios/Core/Log.h>
#include <Helios/Renderer/Renderer.h>

#include "Helios/Core/Application.h"
#include "Helios/Core/Math.h"
#include "Helios/ECSComponents/Components.h"
#include "Helios/Scene/Entity.h"
#include "Helios/Scripting/Script.h"

#include <ImGuizmo.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

float s_Speed = 100.0f;

using namespace Helios;

SandboxLayer::SandboxLayer() {}

SandboxLayer::~SandboxLayer() {}

void SandboxLayer::on_attach() {
    Application& app = Application::get();
    Renderer& renderer = app.get_renderer();

    m_texture = Texture::create(RESOURCES_PATH "images/container2.png");
    m_texture_2 =
        Texture::create(RESOURCES_PATH "images/container2_specular.png");

    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            for (int w = 0; w < 5; w++) {
                Entity cube = m_scene.create_entity("Cube");
                cube.add_component<TransformComponent>(
                    Transform{.position = glm::vec3(i * 2, j * 2, w * 2)});
                cube.add_component<MeshRendererComponent>();
                cube.get_component<MeshRendererComponent>() = {
                    .mesh = renderer.get_cube_mesh(),
                    };
            }
        }
    }

    m_camera = m_scene.create_entity("PerspectiveCamera");
    m_camera.add_component<TransformComponent>(
        Transform{.position = {4.0f, 4.0f, 25.0f}});
    m_camera.add_component<CameraComponent>();

    m_camera_controller = CameraController(
        &m_camera.get_component<TransformComponent>(), 5.0f, 25.0f);

    Entity dir_light = m_scene.create_entity("DirLight");
    dir_light.add_component<DirectionalLightComponent>();
    dir_light.get_component<DirectionalLightComponent>().direction =
        glm::vec3(-1);

    m_viking_mesh = Mesh::create(RESOURCES_PATH "models/viking_room.obj");
    m_viking_texture = Texture::create(RESOURCES_PATH "images/viking_room.png");

    m_viking_room = m_scene.create_entity("Viking room");
    m_viking_room.add_component<TransformComponent>(Transform{
        .position = {5, 2, 15},
        .scale = {2, 2, 2},
    });
    m_viking_room.add_component<MeshRendererComponent>();
    m_viking_room.get_component<MeshRendererComponent>() = {
        .mesh = m_viking_mesh,
    };

    if (FT_Init_FreeType(&m_ft_library)) {
        HL_ERROR("Could not init freetype");
    }

    if (FT_New_Face(m_ft_library, RESOURCES_PATH "fonts/arial.ttf", 0,
                    &m_face_arial)) {
        HL_ERROR("Could not load arial font");
    }
    FT_Set_Pixel_Sizes(m_face_arial, 0, 48);
    if (FT_Load_Char(m_face_arial, 'H', FT_LOAD_RENDER)) {
        HL_ERROR("Could not load char");
    }
}

void SandboxLayer::on_detach() {}

void SandboxLayer::on_update(float ts) {
    m_fps_time_count += ts;
    m_frame_count++;

    // InstancingTest();

    auto& renderer = Application::get().get_renderer();

    renderer.render_text("ELO", {500, 500}, 1.0f, glm::vec4(1.0f));
    renderer.render_text("ELOelo", {700, 500}, 1.0f, glm::vec4(1.0f));
    renderer.submit_ui_quad_instances();

    if (m_fps_time_count >= 1) {
        m_current_fps = (float)m_frame_count / m_fps_time_count;
        m_fps_time_count = 0;
        m_frame_count = 0;
    }
}

void SandboxLayer::on_event(Event& e) { /*m_camera_controller.on_event(e);*/ }

void VerticalSeparator(float paddingY) {
    // get the current cursor position
    ImVec2 p = ImGui::GetCursorScreenPos();
    // Adjust the starting and ending points by paddingY
    ImVec2 start = ImVec2(p.x + ImGui::GetStyle().ItemSpacing.x / 2,
                          p.y + paddingY / 2 + 5);
    ImVec2 end =
        ImVec2(start.x, start.y + ImGui::GetWindowHeight() - paddingY * 2);

    // Draw the line with adjusted start and end points
    ImGui::GetWindowDrawList()->AddLine(start, end,
                                        ImGui::GetColorU32(ImGuiCol_Border));
    // Offset the cursor on the x-axis by the item spacing to simulate the
    // separator width
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
                         ImGui::GetStyle().ItemSpacing.x);
}

void ShowMeshPickerModal() {
    // Use the same unique identifier as used in ImGui::OpenPopup
    if (ImGui::BeginPopupModal("mesh Picker", NULL,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Select a Default mesh:");
        if (ImGui::Button("Cube")) {
            // Load or create a cube mesh
            ImGui::CloseCurrentPopup(); // Close the modal
        }
        ImGui::SameLine();
        if (ImGui::Button("Cylinder")) {
            // Load or create a cylinder mesh
            ImGui::CloseCurrentPopup(); // Close the modal
        }
        // Add more mesh options here

        ImGui::Separator();
        ImGui::Text("Or, load a mesh from file:");

        if (ImGui::Button("Browse")) {
            // Open file dialog
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup(); // Provide a clear option to close
                                        // without action
        }

        ImGui::EndPopup();
    }
}

void SandboxLayer::on_imgui_render() {
    ImGui::Begin("Stats");
    ImGui::Text("FPS: %d", m_current_fps);
    ImGui::End();

    ImGui::Begin("Props");
    ImGui::InputFloat3("position", &m_mesh_transform.position.x);
    ImGui::InputFloat3("rotation", &m_mesh_transform.rotation.x);
    ImGui::InputFloat3("scale", &m_mesh_transform.scale.x);
    ImGui::End();

    std::vector<std::string> loaded_textures;

    for (int i = 0; i < 100; i++) {
        loaded_textures.push_back(std::to_string(i));
    }

    ImGui::Begin("Other");
    // Add your button with the specified width
    if (ImGui::Button("Add Component", ImVec2(50, 0))) {
        ImGui::OpenPopup("mesh Picker");
    }

    ShowMeshPickerModal();
    ImGui::End();

    Renderer& renderer = Application::get().get_renderer();

    ImGuizmo::BeginFrame();
    ImGuizmo::SetOrthographic(false);

    // Set drawlist and correct rect
    ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());
    ImGuizmo::SetRect(Application::get().get_window().getX(),
                      Application::get().get_window().getY(),
                      renderer.get_swapchain()->get_vk_extent().width,
                      renderer.get_swapchain()->get_vk_extent().height);

    auto& transform = m_viking_room.get_component<TransformComponent>();

    glm::mat4 model = transform.to_transform().ToMat4();

    PerspectiveCamera cam(m_camera.get_component<TransformComponent>().to_transform(),
               renderer.get_swapchain()->get_aspect_ratio(),
               120.0f, 0.1f, 100.0f);

    // Debug: Flip Y-axis if needed

    cam.projection_matrix[1][1] *= -1;

    bool use_snap = Input::is_key_pressed(KeyCode::Q);
    glm::vec3 snap = {0.5f, 0.5f, 0.5f};

    // Manipulate gizmo
    ImGuizmo::Manipulate(
        glm::value_ptr(cam.view_matrix), glm::value_ptr(cam.projection_matrix),
        ImGuizmo::TRANSLATE, ImGuizmo::MODE::LOCAL, glm::value_ptr(model),
        nullptr, use_snap ? &snap.x : nullptr);

    // Decompose transform if gizmo is active
    if (ImGuizmo::IsUsing()) {
        Math::decompose_transform(model, transform.position, transform.rotation,
                                  transform.scale);
    }
}

void SandboxLayer::InstancingTest() {
    Renderer& renderer = Application::get().get_renderer();

    PerspectiveCamera cam(Helios::Transform({4.0f, 4.0f, 25.0f}),
               renderer.get_swapchain()->get_aspect_ratio(),
               120.0f, 0.1f, 100.0f);

    renderer.set_perspective_camera(cam);

    renderer.render_directional_light({.direction = glm::vec3(-1)});

    std::vector<MeshRenderingInstance> cubes;

    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            for (int w = 0; w < 5; w++) {
                cubes.push_back(
                    {Transform{.position = glm::vec3(i * 2, j * 2, w * 2)},
                     });
            }
        }
    }

    renderer.draw_cube(cubes);

    /*
    renderer.draw_mesh(m_viking_mesh, {
                              MeshInstance{
                                      .Transform = m_mesh_transform,
                                      .Material = Material{
                                              .diffuse = m_viking_texture
                                      }
                              }
                      });
                                      */

    renderer.submit_mesh_instances({
        .color_load_op = VK_ATTACHMENT_LOAD_OP_CLEAR,
    });
}

void SandboxLayer::RenderAPITest() {}
