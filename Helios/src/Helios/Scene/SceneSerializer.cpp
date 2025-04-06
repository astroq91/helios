#include "SceneSerializer.h"

#include <filesystem>
#include <fstream>
#include <glm/glm.hpp>
#include <variant>
#include <yaml-cpp/yaml.h>

#include "Entity.h"
#include "Helios/Core/Application.h"
#include "Helios/Core/IOUtils.h"
#include "Helios/Core/SharedPtr.h"
#include "Helios/ECSComponents/Components.h"
#include "entt/entity/fwd.hpp"
#include "stduuid/uuid.h"
#include "yaml-cpp/emittermanip.h"
#include "yaml-cpp/exceptions.h"

namespace fs = std::filesystem;

namespace YAML {
template <> struct convert<glm::vec3> {
    static Node encode(const glm::vec3& rhs) {
        Node node;
        node.push_back(rhs.x);
        node.push_back(rhs.y);
        node.push_back(rhs.z);
        return node;
    }

    static bool decode(const Node& node, glm::vec3& rhs) {
        if (!node.IsSequence() || node.size() != 3) {
            return false;
        }

        rhs.x = node[0].as<float>();
        rhs.y = node[1].as<float>();
        rhs.z = node[2].as<float>();
        return true;
    }
};

template <> struct convert<glm::vec4> {
    static Node encode(const glm::vec4& rhs) {
        Node node;
        node.push_back(rhs.x);
        node.push_back(rhs.y);
        node.push_back(rhs.z);
        node.push_back(rhs.w);
        return node;
    }

    static bool decode(const Node& node, glm::vec4& rhs) {
        if (!node.IsSequence() || node.size() != 4) {
            return false;
        }

        rhs.x = node[0].as<float>();
        rhs.y = node[1].as<float>();
        rhs.z = node[2].as<float>();
        rhs.w = node[3].as<float>();
        return true;
    }
};

template <> struct convert<glm::quat> {
    static Node encode(const glm::quat& rhs) {
        Node node;
        node.push_back(rhs.x);
        node.push_back(rhs.y);
        node.push_back(rhs.z);
        node.push_back(rhs.w);
        return node;
    }

    static bool decode(const Node& node, glm::quat& rhs) {
        if (!node.IsSequence() || node.size() != 4) {
            return false;
        }

        rhs.x = node[0].as<float>();
        rhs.y = node[1].as<float>();
        rhs.z = node[2].as<float>();
        rhs.w = node[3].as<float>();
        return true;
    }
};

template <> struct convert<uuids::uuid> {
    static Node encode(const uuids::uuid& rhs) {
        Node node;
        node.push_back(uuids::to_string(rhs));
        return node;
    }

    static bool decode(const Node& node, uuids::uuid& rhs) {
        if (!node.IsScalar()) {
            return false;
        }
        rhs = uuids::uuid::from_string(node.as<std::string>())
                  .value_or(uuids::uuid());
        return true;
    }
};
} // namespace YAML

namespace Helios {
YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& value) {
    out << YAML::Flow;
    out << YAML::BeginSeq << value.x << value.y << value.z << YAML::EndSeq;
    return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& value) {
    out << YAML::Flow;
    out << YAML::BeginSeq << value.x << value.y << value.z << value.w
        << YAML::EndSeq;
    return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const glm::quat& value) {

    out << YAML::Flow;
    out << YAML::BeginSeq << value.x << value.y << value.z << value.w
        << YAML::EndSeq;
    return out;
}

void SceneSerializer::serialize_entity_components(YAML::Emitter& out,
                                                  Entity entity) {
    out << YAML::Key << "components" << YAML::Value;
    out << YAML::BeginMap;
    if (entity.has_component<PersistentIdComponent>()) {
        const auto& component = entity.get_component<PersistentIdComponent>();
        out << YAML::Key << "persistent_id_component" << YAML::Value
            << YAML::BeginMap;
        out << YAML::Key << "id" << YAML::Value
            << uuids::to_string(component.get_id());
        out << YAML::EndMap;
    }
    if (entity.has_component<NameComponent>()) {
        const auto& component = entity.get_component<NameComponent>();
        out << YAML::Key << "name_component" << YAML::Value << YAML::BeginMap;

        out << YAML::Key << "name" << YAML::Value << component.name;

        out << YAML::EndMap;
    }

    if (entity.has_component<TransformComponent>()) {
        const auto& component = entity.get_component<TransformComponent>();
        out << YAML::Key << "transform_component" << YAML::Value
            << YAML::BeginMap;

        out << YAML::Key << "position" << YAML::Value << component.position;
        out << YAML::Key << "rotation" << YAML::Value << component.rotation;

        out << YAML::Key << "scale" << YAML::Value << component.scale;

        out << YAML::EndMap;
    }

    if (entity.has_component<CameraComponent>()) {
        const auto& component = entity.get_component<CameraComponent>();
        out << YAML::Key << "camera_component" << YAML::Value << YAML::BeginMap;

        out << YAML::Key << "fov_y" << YAML::Value << component.fovY;
        out << YAML::Key << "near" << YAML::Value << component.z_near;
        out << YAML::Key << "far" << YAML::Value << component.z_far;

        out << YAML::EndMap;
    }

    if (entity.has_component<DirectionalLightComponent>()) {
        const auto& component =
            entity.get_component<DirectionalLightComponent>();
        out << YAML::Key << "directional_light_component" << YAML::Value
            << YAML::BeginMap;

        out << YAML::Key << "direction" << YAML::Value << component.direction;
        out << YAML::Key << "ambient" << YAML::Value << component.ambient;
        out << YAML::Key << "diffuse" << YAML::Value << component.diffuse;
        out << YAML::Key << "specular" << YAML::Value << component.specular;

        out << YAML::EndMap;
    }

    if (entity.has_component<PointLightComponent>()) {
        const auto& component = entity.get_component<PointLightComponent>();
        out << YAML::Key << "point_light_component" << YAML::Value
            << YAML::BeginMap;

        out << YAML::Key << "position" << YAML::Value << component.position;

        out << YAML::Key << "constant" << YAML::Value << component.constant;
        out << YAML::Key << "linear" << YAML::Value << component.linear;
        out << YAML::Key << "quadratic" << YAML::Value << component.quadratic;

        out << YAML::Key << "ambient" << YAML::Value << component.ambient;
        out << YAML::Key << "diffuse" << YAML::Value << component.diffuse;
        out << YAML::Key << "specular" << YAML::Value << component.specular;

        out << YAML::EndMap;
    }

    if (entity.has_component<MeshRendererComponent>()) {
        const auto& component = entity.get_component<MeshRendererComponent>();
        out << YAML::Key << "mesh_renderer_component" << YAML::Value
            << YAML::BeginMap;

        out << YAML::Key << "mesh";
        if (component.mesh) {
            out << YAML::Value << component.mesh->get_name();
        } else {
            out << YAML::Value << YAML::Null;
        }

        out << YAML::Key << "material";
        if (component.material) {
            out << YAML::Value << component.material->get_name();
        } else {
            out << YAML::Value << YAML::Null;
        }

        out << YAML::Key << "tint_color" << YAML::Value << component.tint_color;

        out << YAML::EndMap;
    }

    if (entity.has_component<RigidBodyComponent>()) {
        const auto& component = entity.get_component<RigidBodyComponent>();
        out << YAML::Key << "rigid_body_component" << YAML::Value
            << YAML::BeginMap;

        out << YAML::Key << "type";
        switch (component.type) {
        case RigidBodyType::Static:
            out << YAML::Value << "static";
            break;
        case RigidBodyType::Dynamic:
            out << YAML::Value << "dynamic";
            break;
        }

        out << YAML::Key << "mass" << YAML::Value << component.mass;
        out << YAML::Key << "kinematic" << YAML::Value << component.kinematic;
        out << YAML::Key << "static_friction" << YAML::Value
            << component.static_friction;
        out << YAML::Key << "dynamic_friction" << YAML::Value
            << component.dynamic_friction;
        out << YAML::Key << "restitution" << YAML::Value
            << component.restitution;
        out << YAML::Key << "override_dynamic_physics" << YAML::Value
            << component.override_dynamic_physics;

        out << YAML::Key << "lock_linear_x" << YAML::Value
            << component.lock_linear_x;
        out << YAML::Key << "lock_linear_y" << YAML::Value
            << component.lock_linear_y;
        out << YAML::Key << "lock_linear_z" << YAML::Value
            << component.lock_linear_z;
        out << YAML::Key << "lock_angular_x" << YAML::Value
            << component.lock_angular_x;
        out << YAML::Key << "lock_angular_y" << YAML::Value
            << component.lock_angular_y;
        out << YAML::Key << "lock_angular_z" << YAML::Value
            << component.lock_angular_z;

        out << YAML::EndMap;
    }

    if (entity.has_component<BoxColliderComponent>()) {
        const auto& component = entity.get_component<BoxColliderComponent>();
        out << YAML::Key << "box_collider_component" << YAML::Value
            << YAML::BeginMap;

        out << YAML::Key << "size" << YAML::Value << component.size;

        out << YAML::EndMap;
    }

    if (entity.has_component<ScriptComponent>()) {
        const auto& component = entity.get_component<ScriptComponent>();
        out << YAML::Key << "script_component" << YAML::Value << YAML::BeginMap;

        out << YAML::Key << "path";
        if (component.script) {
            out << YAML::Value << component.script->get_name();
        } else {
            out << YAML::Value << "";
        }

        out << YAML::Key << "exposed_fields" << YAML::Value << YAML::BeginSeq;
        for (auto& field : component.exposed_fields) {
            out << YAML::BeginMap;

            out << YAML::Key << "field" << YAML::Value << field.name;

            switch (field.type) {
            case ScriptFieldType::Entity: {
                if (std::holds_alternative<uuids::uuid>(field.value)) {
                    auto value = std::get<uuids::uuid>(field.value);
                    out << YAML::Key << "value" << uuids::to_string(value);
                }
                break;
            }
            }

            out << YAML::EndMap;
        }
        out << YAML::EndSeq;

        out << YAML::EndMap;
    }

    out << YAML::EndMap;
}

void SceneSerializer::serialize_entity(YAML::Emitter& out, Entity entity) {
    out << YAML::BeginMap;
    serialize_entity_components(out, entity);
    auto children = m_scene->try_get_entity_children(entity);
    if (children) {
        out << YAML::Key << "children" << YAML::Value << YAML::BeginSeq;
        for (auto it = children->begin(); it != children->end(); ++it) {
            serialize_entity(out, Entity(*it, m_scene));
        }
        out << YAML::EndSeq;
    }
    out << YAML::EndMap;
}

void SceneSerializer::serialize_to_path(const std::filesystem::path& path) {

    std::string buffer;
    serialize_to_string(buffer);

    std::ofstream file;
    file.open(
        IOUtils::resolve_path(Application::get().get_asset_base_path(), path));
    file << buffer.c_str();
    file.close();
}

void SceneSerializer::deserialize_from_path(const std::filesystem::path& path) {
    auto abs_path =
        IOUtils::resolve_path(Application::get().get_asset_base_path(), path);
    if (!std::filesystem::exists(abs_path)) {
        HL_ERROR("Could not deserialize scene: {}, because it does not exist.",
                 path.string());
        return;
    }
    std::ifstream stream(abs_path);
    std::stringstream str_stream;
    str_stream << stream.rdbuf();

    deserialize_from_string(str_stream.str());
}

void SceneSerializer::serialize_to_string(std::string& buffer) {
    YAML::Emitter out;

    out << YAML::BeginMap;
    out << YAML::Key << "entities" << YAML::Value << YAML::BeginSeq;

    // Make sure entities are in the correct order
    m_scene->sort_component<entt::entity>();
    for (auto& entity :
         m_scene->get_view<entt::entity>(entt::exclude<ParentComponent>)) {
        serialize_entity(out,
                         m_scene->get_entity(static_cast<uint32_t>(entity)));
    }

    out << YAML::EndSeq;
    out << YAML::Key << "skybox" << YAML::Value << YAML::BeginMap;
    out << YAML::Key << "enabled" << YAML::Value << m_scene->skybox_enabled();

    auto& custom_skybox = m_scene->get_custom_skybox();
    if (custom_skybox) {
        auto& cube_map_info = custom_skybox->get_cube_map_info();
        out << YAML::Key << "right" << YAML::Value
            << cube_map_info.right.string();
        out << YAML::Key << "left" << YAML::Value
            << cube_map_info.left.string();
        out << YAML::Key << "top" << YAML::Value << cube_map_info.top.string();
        out << YAML::Key << "bottom" << YAML::Value
            << cube_map_info.bottom.string();
        out << YAML::Key << "front" << YAML::Value
            << cube_map_info.front.string();
        out << YAML::Key << "back" << YAML::Value
            << cube_map_info.back.string();
    }

    out << YAML::EndMap;
    out << YAML::EndMap;

    buffer = out.c_str();
}

fs::path resolve_path(const std::string& path) {
    return IOUtils::resolve_path(Application::get().get_asset_base_path(),
                                 fs::path(path));
}

void SceneSerializer::deserialize_from_string(const std::string& buffer) {
    try {
        YAML::Node data = YAML::Load(buffer);
        auto entities = data["entities"];
        if (entities) {
            deserialize_from_string_with_parent(entities, Entity());
        }
        auto skybox = data["skybox"];
        if (skybox && skybox.IsMap()) {
            auto skybox_enabled = skybox["enabled"];
            if (skybox_enabled && skybox_enabled.IsScalar()) {
                m_scene->set_skybox_enabled(skybox_enabled.as<bool>());
            }

            if (m_scene->skybox_enabled()) {
                auto custom_skybox = skybox["custom"];
                if (custom_skybox && custom_skybox.IsMap()) {
                    auto right = custom_skybox["right"];
                    auto left = custom_skybox["left"];
                    auto top = custom_skybox["top"];
                    auto bottom = custom_skybox["bottom"];
                    auto front = custom_skybox["front"];
                    auto back = custom_skybox["back"];
                    if (right && right.IsScalar() && left && left.IsScalar() &&
                        top && top.IsScalar() && bottom && bottom.IsScalar() &&
                        front && front.IsScalar() && back && back.IsScalar()) {
                        auto& asset_manager =
                            Application::get().get_asset_manager();
                        auto maybe_texture =
                            asset_manager.get_texture(right.as<std::string>());
                        if (maybe_texture) {
                            m_scene->set_custom_skybox(maybe_texture);
                        } else {
                            SharedPtr<Texture> texture = Texture::create(
                                {
                                    .right =
                                        resolve_path(right.as<std::string>()),
                                    .left =
                                        resolve_path(left.as<std::string>()),
                                    .top = resolve_path(top.as<std::string>()),
                                    .bottom =
                                        resolve_path(bottom.as<std::string>()),
                                    .front =
                                        resolve_path(front.as<std::string>()),
                                    .back =
                                        resolve_path(back.as<std::string>()),
                                },
                                right.as<std::string>());
                            m_scene->set_custom_skybox(texture);
                            asset_manager.add_texture(texture);
                        }

                    } else {
                        HL_ERROR("Invalid custom skybox for scene");
                    }
                }
            }
        }
    } catch (YAML::Exception& e) {
        HL_ERROR("Failed to parse scene file: {}", e.what());
    }

    m_scene->scene_load_done();
}

void SceneSerializer::deserialize_from_string_with_parent(
    const YAML::Node& entities, Entity parent) {

    Entity deserialized_entity;
    for (auto ent : entities) {
        auto components = ent["components"];
        if (components) {
            std::string name;
            auto name_component = components["name_component"];
            if (name_component) {
                name = name_component["name"].as<std::string>();
            }

            deserialized_entity = m_scene->create_entity(name);

            auto persistent_id_component =
                components["persistent_id_component"];
            if (persistent_id_component) {
                auto uuid = uuids::uuid::from_string(
                    persistent_id_component["id"].as<std::string>(""));
                if (uuid) {
                    auto& pic =
                        deserialized_entity
                            .add_component<PersistentIdComponent>(uuid.value());
                } else {
                    HL_WARN("Invalid uuid for entity: {}", name);
                }
            }

            if (parent != k_no_entity) {
                auto& pc =
                    deserialized_entity.add_component<ParentComponent>(parent);
            }

            auto transform_component = components["transform_component"];
            if (transform_component) {
                auto& tc =
                    deserialized_entity.add_component<TransformComponent>();
                tc.position =
                    transform_component["position"].as<glm::vec3>(tc.position);
                tc.rotation =
                    transform_component["rotation"].as<glm::quat>(tc.rotation);
                tc.scale = transform_component["scale"].as<glm::vec3>(tc.scale);

                m_scene->on_entity_transform_updated(deserialized_entity);
            }

            auto camera_component = components["camera_component"];
            if (camera_component) {
                auto& cc = deserialized_entity.add_component<CameraComponent>();
                cc.fovY = camera_component["fov_y"].as<float>(cc.fovY);
                cc.z_near = camera_component["near"].as<float>(cc.z_near);
                cc.z_far = camera_component["far"].as<float>(cc.z_far);
            }

            auto directional_light_component =
                components["directional_light_component"];
            if (directional_light_component) {
                auto& dlc = deserialized_entity
                                .add_component<DirectionalLightComponent>();
                dlc.direction =
                    directional_light_component["direction"].as<glm::vec3>(
                        dlc.direction);
                dlc.ambient =
                    directional_light_component["ambient"].as<glm::vec3>(
                        dlc.ambient);
                dlc.diffuse =
                    directional_light_component["diffuse"].as<glm::vec3>(
                        dlc.diffuse);
                dlc.specular =
                    directional_light_component["specular"].as<glm::vec3>(
                        dlc.specular);
            }

            auto point_light_component = components["point_light_component"];
            if (point_light_component) {
                auto& plc =
                    deserialized_entity.add_component<PointLightComponent>();
                plc.position = point_light_component["position"].as<glm::vec3>(
                    plc.position);

                plc.constant =
                    point_light_component["constant"].as<float>(plc.constant);
                plc.linear =
                    point_light_component["linear"].as<float>(plc.linear);
                plc.quadratic =
                    point_light_component["quadratic"].as<float>(plc.quadratic);

                plc.ambient =
                    point_light_component["ambient"].as<glm::vec3>(plc.ambient);
                plc.diffuse =
                    point_light_component["diffuse"].as<glm::vec3>(plc.diffuse);
                plc.specular = point_light_component["specular"].as<glm::vec3>(
                    plc.specular);
            }

            auto mesh_renderer_component =
                components["mesh_renderer_component"];
            if (mesh_renderer_component) {
                auto& mc =
                    deserialized_entity.add_component<MeshRendererComponent>();

                auto mesh = mesh_renderer_component["mesh"];
                if (mesh && !mesh.IsNull() && mesh.IsScalar()) {
                    std::string mesh_name = mesh.as<std::string>();

                    if (mesh_name == "Cube") {
                        mc.mesh =
                            Application::get().get_renderer().get_cube_mesh();
                    }
                    // Some file path
                    else if (!mesh_name.empty()) {
                        auto mesh =
                            Application::get().get_asset_manager().get_mesh(
                                mesh_name);
                        if (mesh == nullptr) {
                            mc.mesh =
                                Mesh::create(std::filesystem::path(mesh_name));
                            if (mc.mesh) {
                                Application::get().get_asset_manager().add_mesh(
                                    mc.mesh);
                            }
                        } else {
                            mc.mesh = mesh;
                        }
                    }
                }

                auto material = mesh_renderer_component["material"];
                if (material && !material.IsNull() && material.IsScalar()) {
                    std::string material_path = material.as<std::string>();
                    if (!material_path.empty()) {
                        auto mat =
                            Application::get().get_asset_manager().get_material(
                                material_path);
                        if (mat == nullptr) {
                            mc.material = Material::create(
                                std::filesystem::path(material_path));
                            if (mc.material) {
                                Application::get()
                                    .get_asset_manager()
                                    .add_material(mc.material);
                            }
                        } else {
                            mc.material = mat;
                        }
                    }
                }

                auto tint_color = mesh_renderer_component["tint_color"];
                if (tint_color && !tint_color.IsNull()) {
                    mc.tint_color = tint_color.as<glm::vec4>();
                }
            }

            auto rb_component = components["rigid_body_component"];
            if (rb_component) {
                auto& rb =
                    deserialized_entity.add_component<RigidBodyComponent>();
                std::string type = rb_component["type"].as<std::string>();
                if (type == "static") {
                    rb.type = RigidBodyType::Static;
                } else if (type == "dynamic") {
                    rb.type = RigidBodyType::Dynamic;
                }

                rb.mass = rb_component["mass"].as<float>(rb.mass);
                rb.kinematic = rb_component["kinematic"].as<bool>(rb.kinematic);
                rb.static_friction = rb_component["static_friction"].as<float>(
                    rb.static_friction);
                rb.dynamic_friction =
                    rb_component["dynamic_friction"].as<float>(
                        rb.dynamic_friction);
                rb.restitution =
                    rb_component["restitution"].as<float>(rb.restitution);
                rb.override_dynamic_physics =
                    rb_component["override_dynamic_physics"].as<bool>(
                        rb.override_dynamic_physics);
                rb.lock_linear_x =
                    rb_component["lock_linear_x"].as<bool>(rb.lock_linear_x);
                rb.lock_linear_y =
                    rb_component["lock_linear_y"].as<bool>(rb.lock_linear_y);
                rb.lock_linear_z =
                    rb_component["lock_linear_z"].as<bool>(rb.lock_linear_z);
                rb.lock_angular_x =
                    rb_component["lock_angular_x"].as<bool>(rb.lock_angular_x);
                rb.lock_angular_y =
                    rb_component["lock_angular_y"].as<bool>(rb.lock_angular_y);
                rb.lock_angular_z =
                    rb_component["lock_angular_z"].as<bool>(rb.lock_angular_z);
            }

            auto bc_component = components["box_collider_component"];
            if (bc_component) {
                auto& bc =
                    deserialized_entity.add_component<BoxColliderComponent>();
                bc.size = bc_component["size"].as<glm::vec3>(bc.size);
            }

            auto script_component = components["script_component"];
            if (script_component) {
                auto& sc = deserialized_entity.add_component<ScriptComponent>();
                std::string path = script_component["path"].as<std::string>();
                if (!script_component["path"].IsNull() && !path.empty()) {
                    sc.script =
                        std::make_unique<Script>(path, ScriptLoadType::File,
                                                 m_scene, deserialized_entity);
                }

                auto fields = script_component["exposed_fields"];
                if (!fields.IsNull() && fields.IsSequence()) {
                    for (auto field : fields) {
                        auto name = field["field"];
                        auto value = field["value"];
                        if (name.IsNull() || !name.IsScalar()) {
                            // Valid field name, and type, is needed
                            HL_WARN("Invalid field type or name, when loading "
                                    "scene");
                            continue;
                        }

                        for (auto& field_data : sc.exposed_fields) {
                            if (field_data.name == name.as<std::string>()) {
                                if (!value.IsNull() && value.IsScalar()) {
                                    auto script_field =
                                        sc.script->get_exposed_field(
                                            name.as<std::string>());

                                    if (!script_field) {
                                        HL_ERROR(
                                            "Could not find script field: {}",
                                            name.as<std::string>());
                                        continue;
                                    }

                                    switch (script_field->get_type()) {
                                    case ScriptFieldType::Entity: {
                                        field_data.value =
                                            value.as<uuids::uuid>();
                                        break;
                                    }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        auto children = ent["children"];
        if (children && deserialized_entity != k_no_entity) {
            deserialize_from_string_with_parent(children, deserialized_entity);
        }
    }
}

} // namespace Helios
