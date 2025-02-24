#include "ComponentsBrowser.h"
#include <functional>
#include <imgui.h>
#include <string>

#include "Helios/ECSComponents/Components.h"
#include "Helios/Physics/PhysicsManager.h"
#include "Helios/Renderer/Renderer.h"
#include "Helios/Scene/Entity.h"
#include "Helios/Scene/Scene.h"

#include "Helios/Core/Application.h"
#include "Helios/Core/IOUtils.h"

enum class TextureType { Diffuse, Specular, Emission };

namespace Utils {
/**
 * \brief Function for rendering a component in ImGui.
 * \tparam Type The component type.
 * \param component_name The name of the component (displayed in the editor).
 * \param registry The entt registry.
 * \param selected_entity The selected entity.
 * \param callback Where you put ImGui items to control the component data.
 */
template <typename Type>
void render_component(const std::string& component_name,
                      Helios::Entity selected_entity,
                      const std::function<void(Type*)>& callback) {
    auto component = selected_entity.try_get_component<Type>();
    if (component) {
        ImGui::Separator();

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::TreeNode(component_name.c_str())) {
            if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                ImGui::OpenPopup("Remove");
            }

            if (ImGui::BeginPopup("Remove")) {
                if (ImGui::MenuItem("Remove Component")) {
                    selected_entity.remove_component<Type>();
                }
                ImGui::EndPopup();
            }

            // Handle the rest in the callback
            callback(component);

            ImGui::TreePop();
        }
    }
}

void draw_custom_divider(float thickness, ImVec4 color) {
    ImVec2 pos = ImGui::GetCursorScreenPos();
    pos.y += ImGui::GetStyle().FramePadding.y;

    ImGui::GetWindowDrawList()->AddLine(
        ImVec2(pos.x, pos.y),
        ImVec2(pos.x + ImGui::GetContentRegionAvail().x -
                   ImGui::GetStyle().FramePadding.x * 2,
               pos.y),
        ImGui::GetColorU32(color), thickness);

    ImGui::GetWindowDrawList()->AddLine(
        ImVec2(pos.x + ImGui::GetContentRegionAvail().x -
                   ImGui::GetStyle().FramePadding.x * 2,
               pos.y),
        ImVec2(pos.x + ImGui::GetContentRegionAvail().x -
                   ImGui::GetStyle().FramePadding.x * 2,
               pos.y + ImGui::GetContentRegionAvail().y -
                   ImGui::GetStyle().FramePadding.y * 2),
        ImGui::GetColorU32(color), thickness);

    ImGui::Dummy(ImVec2(0.0f, thickness));
}

glm::vec3 clamp_euler(glm::vec3 euler) {
    euler.x = glm::mod(euler.x + 180.0f, 360.0f) - 180.0f;
    euler.y = glm::mod(euler.y + 180.0f, 360.0f) - 180.0f;
    euler.z = glm::mod(euler.z + 180.0f, 360.0f) - 180.0f;
    return euler;
}

} // namespace Utils

namespace Helios {
void ComponentsBrowser::on_update(Scene* scene, Entity selected_entity,
                                  const Project& project) {
    Renderer& renderer = Application::get().get_renderer();
    ImGui::Begin("Components Browser");
    // If we have a selected entity
    if (selected_entity != k_no_entity) {
        auto name_component =
            selected_entity.try_get_component<NameComponent>();
        if (name_component) {
            static char buffer[256];
            strcpy(buffer, name_component->name.c_str());

            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            if (ImGui::TreeNode("name")) {
                ImGui::InputText("name", buffer, sizeof(buffer));

                ImGui::TreePop();

                name_component->name = buffer;
            }
        }

        Utils::render_component<TransformComponent>(
            "Transform", selected_entity, [&](auto component) {
                static glm::vec3 eulerCache;
                static glm::quat
                    lastRotation; // Tracks the last known quaternion
                static TransformComponent* lastComponent = nullptr;

                if (component != lastComponent ||
                    component->rotation != lastRotation) {
                    eulerCache = component->get_euler(); // Refresh Euler cache
                    lastRotation =
                        component->rotation; // Update last known rotation
                    lastComponent =
                        component; // Track the currently edited component
                }

                auto rb =
                    selected_entity.try_get_component<RigidBodyComponent>();
                bool disable_transform =
                    scene->is_running() && rb->type == RigidBodyType::Dynamic &&
                    !rb->kinematic && !rb->override_dynamic_physics;

                if (disable_transform) {
                    ImGui::BeginDisabled();
                }

                ImGui::InputFloat3("position", &component->position.x);

                if (ImGui::InputFloat3("rotation", &eulerCache.x)) {
                    eulerCache = Utils::clamp_euler(
                        eulerCache); // Clamp Euler angles to [-180, 180]
                    component->set_euler(eulerCache); // Update quaternion
                    lastRotation =
                        component->rotation; // Sync last known rotation
                }

                ImGui::InputFloat3("scale", &component->scale.x);

                if (disable_transform) {
                    ImGui::EndDisabled();
                }
            });

        Utils::render_component<CameraComponent>(
            "Camera", selected_entity, [](auto component) {
                ImGui::InputFloat("fovY", &component->fovY);
                ImGui::InputFloat("near", &component->near);
                ImGui::InputFloat("far", &component->far);
            });

        Utils::render_component<DirectionalLightComponent>(
            "Directional Light", selected_entity, [](auto component) {
                ImGui::InputFloat3("direction", &component->direction.x);
                ImGui::InputFloat3("ambient", &component->ambient.x);
                ImGui::InputFloat3("diffuse", &component->diffuse.x);
                ImGui::InputFloat3("specular", &component->specular.x);
            });

        Utils::render_component<PointLightComponent>(
            "Point Light", selected_entity, [](auto component) {
                ImGui::InputFloat3("position", &component->position.x);
                ImGui::InputFloat3("ambient", &component->ambient.x);
                ImGui::InputFloat3("diffuse", &component->diffuse.x);
                ImGui::InputFloat3("specular", &component->specular.x);

                ImGui::InputFloat("constant", &component->constant);
                ImGui::InputFloat("linear", &component->linear);
                ImGui::InputFloat("quadratic", &component->quadratic);
            });

        Utils::render_component<MeshComponent>(
            "Mesh", selected_entity,
            [&renderer, &project](auto component) {
                static bool show_choose_mesh_modal = false;

                char mesh_name_buffer[256];
                char diffuse_buffer[256];
                char specular_buffer[256];
                char emission_buffer[256];

                if (ImGui::TreeNode("Mesh")) {
                    if (component->mesh) {
                        strcpy(mesh_name_buffer,
                               component->mesh->get_name().c_str());
                    } else {
                        strcpy(mesh_name_buffer, "None");
                    }

                    if (ImGui::Button("Choose")) {
                        show_choose_mesh_modal = true;
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("Clear")) {
                        component->mesh = nullptr;
                    }

                    ImGui::SameLine();
                    ImGui::Text("(Current: %s)", mesh_name_buffer);

                    if (show_choose_mesh_modal) {
                        ImGui::OpenPopup("Choose mesh");
                        if (ImGui::BeginPopupModal(
                                "Choose mesh", &show_choose_mesh_modal,
                                ImGuiWindowFlags_AlwaysAutoResize)) {
                            ImGui::Text("Select a Default mesh:");
                            if (ImGui::Button("Cube")) {
                                component->mesh = renderer.get_cube_mesh();
                                show_choose_mesh_modal = false;
                            }
                            // Add more mesh options here

                            ImGui::Separator();
                            ImGui::Text("Or, load a mesh from file:");

                            if (ImGui::Button("Browse")) {
                                DialogReturn dialogRet = IOUtils::open_file(
                                    {"*.obj"}, project.get_project_path());
                                if (!dialogRet.path.empty()) {
                                    std::string relative_path =
                                        IOUtils::relative_path(
                                            project.get_project_path(),
                                            dialogRet.path);

                                    auto mesh = Application::get()
                                                    .get_asset_manager()
                                                    .get_mesh(relative_path);
                                    if (mesh == nullptr) {
                                        component->mesh =
                                            Mesh::create(relative_path);
                                        Application::get()
                                            .get_asset_manager()
                                            .add_mesh(component->mesh);
                                    } else {
                                        component->mesh = mesh;
                                    }

                                    show_choose_mesh_modal = false;
                                }
                            }

                            ImGui::SameLine();
                            if (ImGui::Button("Cancel")) {
                                show_choose_mesh_modal = false;
                            }

                            ImGui::EndPopup();
                        }
                    }

                    ImGui::TreePop();
                }

                // TODO: More mesh types?

                TextureType currentTextureType = TextureType::Diffuse;
                if (ImGui::TreeNode("Material")) {
                    if (component->material.diffuse) {
                        strcpy(diffuse_buffer,
                               component->material.diffuse->get_name().c_str());
                    } else {
                        strcpy(diffuse_buffer, "None");
                    }

                    if (component->material.specular) {
                        strcpy(
                            specular_buffer,
                            component->material.specular->get_name().c_str());
                    } else {
                        strcpy(specular_buffer, "None");
                    }

                    if (component->material.emission) {
                        strcpy(
                            emission_buffer,
                            component->material.emission->get_name().c_str());
                    } else {
                        strcpy(emission_buffer, "None");
                    }

                    // DIFFUSE
                    ImGui::PushID("diffuse");
                    if (ImGui::Button("Choose")) {
                        DialogReturn dialog_ret =
                            IOUtils::open_file({"*.png", "*.jpg", "*.jpeg"},
                                               project.get_project_path());
                        if (!dialog_ret.path.empty()) {
                            std::string relative_path = IOUtils::relative_path(
                                project.get_project_path(), dialog_ret.path);

                            auto texture = Application::get()
                                               .get_asset_manager()
                                               .get_texture(relative_path);
                            if (texture == nullptr) {
                                component->material.diffuse =
                                    Texture::create(relative_path);
                                Application::get()
                                    .get_asset_manager()
                                    .add_texture(component->material.diffuse);
                            } else {
                                component->material.diffuse = texture;
                            }
                        }
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("Clear")) {
                        component->material.diffuse = nullptr;
                    }
                    ImGui::SameLine();
                    ImGui::Text("diffuse (Current: %s)", diffuse_buffer);
                    ImGui::PopID();

                    ImGui::NewLine();

                    // SPECULAR
                    ImGui::PushID("specular");
                    if (ImGui::Button("Choose")) {
                        DialogReturn dialog_ret =
                            IOUtils::open_file({"*.png", "*.jpg", "*.jpeg"},
                                               project.get_project_path());
                        if (!dialog_ret.path.empty()) {
                            std::string relative_path = IOUtils::relative_path(
                                project.get_project_path(), dialog_ret.path);

                            auto texture = Application::get()
                                               .get_asset_manager()
                                               .get_texture(relative_path);
                            if (texture == nullptr) {
                                component->material.specular =
                                    Texture::create(relative_path);
                                Application::get()
                                    .get_asset_manager()
                                    .add_texture(component->material.specular);
                            } else {
                                component->material.specular = texture;
                            }
                        }
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Clear")) {
                        component->material.specular = nullptr;
                    }
                    ImGui::SameLine();
                    ImGui::Text("specular (Current: %s)", specular_buffer);
                    ImGui::PopID();

                    ImGui::NewLine();

                    // EMISSION
                    ImGui::PushID("emission");
                    if (ImGui::Button("Choose")) {
                        DialogReturn dialog_ret =
                            IOUtils::open_file({"*.png", "*.jpg", "*.jpeg"},
                                               project.get_project_path());
                        if (!dialog_ret.path.empty()) {
                            std::string relative_path = IOUtils::relative_path(
                                project.get_project_path(), dialog_ret.path);

                            auto texture = Application::get()
                                               .get_asset_manager()
                                               .get_texture(relative_path);
                            if (texture == nullptr) {
                                component->material.emission =
                                    Texture::create(relative_path);
                                Application::get()
                                    .get_asset_manager()
                                    .add_texture(component->material.emission);
                            } else {
                                component->material.emission = texture;
                            }
                        }
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Clear")) {
                        component->material.emission = nullptr;
                    }
                    ImGui::SameLine();
                    ImGui::Text("emission (Current: %s)", emission_buffer);
                    ImGui::PopID();

                    ImGui::TreePop();
                }

                // Choose Texture Modal
            }

        );

        Utils::render_component<RigidBodyComponent>(
            "Rigid Body", selected_entity, [&](auto component) {
                int current_type = static_cast<int>(component->type);

                if (scene->is_running()) {
                    ImGui::BeginDisabled();
                }
                const char* rb_types[] = {"Dynamic", "Static"};
                if (ImGui::BeginCombo("Type", rb_types[current_type])) {
                    // Check if this item is selected
                    if (ImGui::Selectable(rb_types[0], current_type == 0)) {
                        component->type = RigidBodyType::Dynamic;
                        ImGui::SetItemDefaultFocus();
                    }

                    if (ImGui::Selectable(rb_types[1], current_type == 1)) {
                        component->type = RigidBodyType::Static;
                        ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }

                if (scene->is_running()) {
                    ImGui::EndDisabled();
                }

                ImGui::Separator();

                if (ImGui::InputFloat("Mass", &component->mass)) {
                    scene->update_rigid_body_mass(selected_entity,
                                                  component->mass);
                }
                if (ImGui::InputFloat("Static Friction",
                                      &component->static_friction)) {
                    scene->update_rigid_body_static_friction(
                        selected_entity, component->static_friction);
                }
                if (ImGui::InputFloat("Dynamic Friction",
                                      &component->dynamic_friction)) {
                    scene->update_rigid_body_dynamic_friction(
                        selected_entity, component->dynamic_friction);
                }
                if (ImGui::InputFloat("Restitution", &component->restitution)) {
                    scene->update_rigid_body_restitution(
                        selected_entity, component->restitution);
                }

                if (current_type == 0) {
                    ImGui::Separator();
                    if (ImGui::Checkbox("Kinematic", &component->kinematic)) {
                        if (component->kinematic) {
                            component->override_dynamic_physics = false;
                        }
                    }

                    if (component->kinematic) {
                        ImGui::BeginDisabled();
                    }
                    ImGui::Checkbox("Override Dynamic Physics",
                                    &component->override_dynamic_physics);
                    if (component->kinematic) {
                        ImGui::EndDisabled();
                    }
                }
            });

        Utils::render_component<BoxColliderComponent>(
            "Box Collider", selected_entity, [](auto component) {
                ImGui::InputFloat3("Size", &component->size.x);
            });

        Utils::render_component<ScriptComponent>(
            "Script", selected_entity,
            [&project, selected_entity](auto component) {
                char script_name_buffer[256];

                if (component->script) {
                    strcpy(script_name_buffer,
                           component->script->get_name().c_str());
                } else {
                    strcpy(script_name_buffer, "None");
                }

                if (ImGui::Button("Choose")) {
                    DialogReturn dialogRet = IOUtils::open_file(
                        {"*.lua"}, project.get_project_path());
                    if (!dialogRet.path.empty()) {
                        std::string relative_path = IOUtils::relative_path(
                            project.get_project_path(), dialogRet.path);
                        component->script = std::make_unique<Script>(
                            dialogRet.path, selected_entity,
                            ScriptLoadType::File);
                    }
                }

                ImGui::SameLine();
                if (ImGui::Button("Clear")) {
                    component->script = nullptr;
                }

                ImGui::SameLine();
                ImGui::Text("(Current: %s)", script_name_buffer);
            });

        ImGui::Separator();

        ImGui::Dummy(ImVec2(0.0f, 5.0f));

        float button_width = 150.0f;
        float window_width = ImGui::GetWindowWidth();
        float x_pos = (window_width - button_width) * 0.5f;

        // Place the button at the calculated position
        ImGui::SetCursorPosX(x_pos);

        // Add your button with the specified width
        if (ImGui::Button("Add Component", ImVec2(button_width, 0))) {
            m_show_add_component_modal = true;
        }

        if (m_show_add_component_modal) {
            ImGui::OpenPopup("Add Component");
            if (ImGui::BeginPopupModal("Add Component",
                                       &m_show_add_component_modal,
                                       ImGuiWindowFlags_AlwaysAutoResize)) {
                if (!selected_entity.try_get_component<TransformComponent>() &&
                    ImGui::Button("Transform")) {
                    selected_entity.add_component<TransformComponent>();
                    m_show_add_component_modal = false;
                }

                if (!selected_entity.try_get_component<CameraComponent>() &&
                    ImGui::Button("Camera")) {
                    selected_entity.add_component<CameraComponent>();
                    m_show_add_component_modal = false;
                }

                if (!selected_entity.try_get_component<MeshComponent>() &&
                    ImGui::Button("Mesh")) {
                    selected_entity.add_component<MeshComponent>();
                    m_show_add_component_modal = false;
                }

                if (!selected_entity
                         .try_get_component<DirectionalLightComponent>() &&
                    ImGui::Button("Directional Light")) {
                    selected_entity.add_component<DirectionalLightComponent>();
                    m_show_add_component_modal = false;
                }

                if (!selected_entity.try_get_component<PointLightComponent>() &&
                    ImGui::Button("Point Light")) {
                    selected_entity.add_component<PointLightComponent>();
                    m_show_add_component_modal = false;
                }

                if (!selected_entity.try_get_component<RigidBodyComponent>() &&
                    ImGui::Button("Rigid Body")) {
                    selected_entity.add_component<RigidBodyComponent>();
                    m_show_add_component_modal = false;
                }

                if (!selected_entity
                         .try_get_component<BoxColliderComponent>() &&
                    ImGui::Button("Box Collider")) {
                    selected_entity.add_component<BoxColliderComponent>();
                    m_show_add_component_modal = false;
                }

                if (!selected_entity.try_get_component<ScriptComponent>() &&
                    ImGui::Button("Script")) {
                    selected_entity.add_component<ScriptComponent>();
                    m_show_add_component_modal = false;
                }

                ImGui::EndPopup();
            }
        }
    }
    ImGui::End();
}
} // namespace Helios
