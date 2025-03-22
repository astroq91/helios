#include "Script.h"
#include "Helios/Core/Application.h"
#include "Helios/Core/IOUtils.h"
#include "Helios/Core/Log.h"
#include "Helios/ECSComponents/Components.h"
#include "Helios/Events/Input.h"
#include "Helios/Scene/Entity.h"
#include "Helios/Scene/Scene.h"

#include "Helios/Scripting/ScriptUserTypes/Entities.h"
#include "Helios/Scripting/ScriptUserTypes/Entity.h"
#include "Helios/Scripting/ScriptUserTypes/MeshRenderer.h"

#include "Helios/Scripting/ScriptUserTypes/UI.h"
#include "sol/property.hpp"
#include "sol/variadic_args.hpp"
#include <fstream>
#include <glm/glm.hpp>
#include <sstream>
#include <utility>

namespace Helios {

Script::Script(const std::string& src, ScriptLoadType load_type, Scene* scene,
               Entity entity)
    : m_scene(scene), m_entity(entity) {
    m_state.open_libraries(sol::lib::base, sol::lib::table, sol::lib::package,
                           sol::lib::math, sol::lib::os);

    if (load_type == ScriptLoadType::File) {
        init_asset(src);
    } else {
        init_asset("Loaded from source");
    }

    expose_component_user_types();
    expose_key_codes();
    expose_functions();
    expose_basic_types();
    expose_helios_user_types();
    set_globals();
    // load_global_fields();

    // Add the base dir to the package path to access the type annotations
    std::stringstream ss;
    ss << "package.path = package.path .. ';" << BASE_DIR << "?.lua'";

    m_state.script(ss.str());

    load_script(src, load_type);
}

void Script::load_script(const std::string& src, ScriptLoadType load_type) {
    if (load_type == ScriptLoadType::Source) {
        m_state.script(src);
        parse_exposed_fields(src);
    } else {
        std::ifstream file(
            IOUtils::resolve_path(Application::get().get_asset_base_path(),
                                  std::filesystem::path(src)),
            std::ios::ate | std::ios::binary);

        if (file.fail()) {
            HL_ERROR("[Scripting] Could not open script file: {0}", src);
            return;
        }

        size_t file_size = (size_t)file.tellg();
        std::vector<char> src(file_size);
        file.seekg(0);
        file.read(src.data(), file_size);
        file.close();

        std::string script(src.begin(), src.end());

        try {

            m_state.script(script);
        } catch (sol::error& error) {
            HL_ERROR("Failed to parse script ({}): {}", src.data(),
                     error.what());
        }

        parse_exposed_fields(script);
    }
}

void Script::on_start() {
    auto func = m_state["on_start"];
    if (func.valid()) {
        try {
            func();
        } catch (sol::error err) {
            HL_ERROR("[Scripting] Error invoking on_start, for script '{}': {} "
                     "(entity: '{}', "
                     "id: {})",
                     m_name, err.what(),
                     m_entity.get_component<NameComponent>().name,
                     static_cast<uint32_t>(m_entity));
        }
    }
}

void Script::on_update(float ts) {
    set_exposed_fields_state();
    auto func = m_state["on_update"];
    if (func.valid()) {
        try {
            func(ts);
        } catch (sol::error err) {
            HL_ERROR(
                "[Scripting] Error invoking on_update, for script '{}': {} "
                "(entity: '{}', "
                "id: {})",
                m_name, err.what(),
                m_entity.get_component<NameComponent>().name,
                static_cast<uint32_t>(m_entity));
        }
    }
    get_exposed_fields_state();
}

void Script::expose_basic_types() {
    m_state.new_usertype<glm::vec2>(
        "Vec2", sol::constructors<glm::vec2(), glm::vec2(float, float)>(), "x",
        &glm::vec2::x, "y", &glm::vec2::y);
    m_state.new_usertype<glm::vec3>(
        "Vec3",
        sol::constructors<glm::vec3(), glm::vec3(float, float, float)>(), "x",
        &glm::vec3::x, "y", &glm::vec3::y, "z", &glm::vec3::z);
    m_state.new_usertype<glm::vec4>(
        "Vec4",
        sol::constructors<glm::vec4(), glm::vec4(float, float, float, float)>(),
        "x", &glm::vec4::x, "y", &glm::vec4::y, "z", &glm::vec4::z, "w",
        &glm::vec4::w);
    m_state.new_usertype<glm::quat>("Quat", "x", &glm::quat::x, "y",
                                    &glm::quat::y, "z", &glm::quat::z, "w",
                                    &glm::quat::w);
}

void Script::expose_functions() {
    m_state["print"] = [&](sol::variadic_args args) {
        std::stringstream lua_output;
        for (auto arg : args) {
            lua_output << arg.as<std::string>() << "\t";
        }
        HL_INFO("[Script] {}", lua_output.str());
    };
}

void Script::expose_helios_user_types() {
    m_state.new_usertype<ScriptUserTypes::ScriptEntity>(
        "Entity", sol::constructors<ScriptUserTypes::ScriptEntity()>(),
        "get_components", &ScriptUserTypes::ScriptEntity::get_components);

    m_state.new_usertype<ScriptUserTypes::ScriptEntities>(
        "Entities", "create_entity",
        &ScriptUserTypes::ScriptEntities::create_entity);
    m_state.new_usertype<ScriptUserTypes::ScriptComponents>(
        "ScriptUserTypes::ScriptComponents", "get_name",
        &ScriptUserTypes::ScriptComponents::get_name, "get_transform",
        &ScriptUserTypes::ScriptComponents::get_transform, "add_transform",
        &ScriptUserTypes::ScriptComponents::add_transform, "get_camera",
        &ScriptUserTypes::ScriptComponents::get_camera, "add_camera",
        &ScriptUserTypes::ScriptComponents::add_camera, "get_directional_light",
        &ScriptUserTypes::ScriptComponents::get_directional_light,
        "add_directional_light",
        &ScriptUserTypes::ScriptComponents::add_directional_light,
        "get_point_light", &ScriptUserTypes::ScriptComponents::get_point_light,
        "add_point_light", &ScriptUserTypes::ScriptComponents::add_point_light,
        "get_rigid_body", &ScriptUserTypes::ScriptComponents::get_rigid_body,
        "add_rigid_body", &ScriptUserTypes::ScriptComponents::add_rigid_body,
        "get_mesh_renderer",
        &ScriptUserTypes::ScriptComponents::get_mesh_renderer,
        "add_mesh_renderer",
        &ScriptUserTypes::ScriptComponents::add_mesh_renderer);

    m_state.new_usertype<Input>("Input", "is_key_pressed",
                                &Input::is_key_pressed, "is_key_released",
                                &Input::is_key_released);
    m_state.new_usertype<ScriptUserTypes::ScriptUI>(
        "UI", "render_text", &ScriptUserTypes::ScriptUI::render_text,
        "get_window_width", &ScriptUserTypes::ScriptUI::get_window_width,
        "get_window_height", &ScriptUserTypes::ScriptUI::get_window_height);
}

void Script::expose_component_user_types() {

    m_state.new_usertype<TransformComponent>(
        "Transform", "position", &TransformComponent::position, "rotation",
        &TransformComponent::rotation, "scale", &TransformComponent::scale);

    m_state.new_usertype<NameComponent>("Name", "name", &NameComponent::name);

    m_state.new_usertype<CameraComponent>(
        "Camera", "fov_y", &CameraComponent::fovY, "near",
        &CameraComponent::z_near, "far", &CameraComponent::z_far);

    m_state.new_usertype<DirectionalLightComponent>(
        "DirectionalLight", "direction", &DirectionalLightComponent::direction,
        "ambient", &DirectionalLightComponent::ambient, "diffuse",
        &DirectionalLightComponent::diffuse, "specular",
        &DirectionalLightComponent::specular);

    m_state.new_usertype<PointLightComponent>(
        "PointLight", "position", &PointLightComponent::position, "constant",
        &PointLightComponent::constant, "linear", &PointLightComponent::linear,
        "quadratic", &PointLightComponent::quadratic, "ambient",
        &PointLightComponent::ambient, "diffuse", &PointLightComponent::diffuse,
        "specular", &PointLightComponent::specular);

    m_state.new_usertype<RigidBodyComponent>(
        "RigidBody", "mass",
        sol::property(
            [&](RigidBodyComponent& component) { return component.mass; },
            [&](RigidBodyComponent& component, float value) {
                component.mass = value;
                m_entity.update_rigid_body_mass(value);
            }),
        "static_friction",
        sol::property(
            [&](RigidBodyComponent& component) {
                return component.static_friction;
            },
            [&](RigidBodyComponent& component, float value) {
                component.static_friction = value;
                m_entity.update_rigid_body_static_friction(value);
            }),
        "dynamic_friction",
        sol::property(
            [&](RigidBodyComponent& component) {
                return component.dynamic_friction;
            },
            [&](RigidBodyComponent& component, float value) {
                component.dynamic_friction = value;
                m_entity.update_rigid_body_dynamic_friction(value);
            }),
        "restitution",
        sol::property(
            [&](RigidBodyComponent& component) {
                return component.restitution;
            },
            [&](RigidBodyComponent& component, float value) {
                component.restitution = value;
                m_entity.update_rigid_body_restitution(value);
            }));

    m_state.new_usertype<ScriptUserTypes::ScriptMeshRenderer>(
        "MeshRenderer", "load_mesh",
        &ScriptUserTypes::ScriptMeshRenderer::load_mesh, "load_material",
        &ScriptUserTypes::ScriptMeshRenderer::load_material, "tint_color",
        sol::property(
            [&](ScriptUserTypes::ScriptMeshRenderer& mesh) {
                return mesh.get_component()->tint_color;
            },
            [&](ScriptUserTypes::ScriptMeshRenderer& mesh, glm::vec4 color) {
                mesh.get_component()->tint_color = color;
            }));
}

void Script::expose_key_codes() {
    m_state.new_enum(
        "Key", "Space", Key::Space, "Apostrophe", Key::Apostrophe, "Comma",
        Key::Comma, "Minus", Key::Minus, "Period", Key::Period, "Slash",
        Key::Slash, "D0", Key::D0, "D1", Key::D1, "D2", Key::D2, "D3", Key::D3,
        "D4", Key::D4, "D5", Key::D5, "D6", Key::D6, "D7", Key::D7, "D8",
        Key::D8, "D9", Key::D9, "Semicolon", Key::Semicolon, "Equal",
        Key::Equal, "A", Key::A, "B", Key::B, "C", Key::C, "D", Key::D, "E",
        Key::E, "F", Key::F, "G", Key::G, "H", Key::H, "I", Key::I, "J", Key::J,
        "K", Key::K, "L", Key::L, "M", Key::M, "N", Key::N, "O", Key::O, "P",
        Key::P, "Q", Key::Q, "R", Key::R, "S", Key::S, "T", Key::T, "U", Key::U,
        "V", Key::V, "W", Key::W, "X", Key::X, "Y", Key::Y, "Z", Key::Z,
        "LeftBracket", Key::LeftBracket, "Backslash", Key::Backslash,
        "RightBracket", Key::RightBracket, "GraveAccent", Key::GraveAccent,
        "World1", Key::World1, "World2", Key::World2, "Escape", Key::Escape,
        "Enter", Key::Enter, "Tab", Key::Tab, "Backspace", Key::Backspace,
        "Insert", Key::Insert, "Delete", Key::Delete, "Right", Key::Right,
        "Left", Key::Left, "Down", Key::Down, "Up", Key::Up, "PageUp",
        Key::PageUp, "PageDown", Key::PageDown, "Home", Key::Home, "End",
        Key::End, "CapsLock", Key::CapsLock, "ScrollLock", Key::ScrollLock,
        "NumLock", Key::NumLock, "PrintScreen", Key::PrintScreen, "Pause",
        Key::Pause, "F1", Key::F1, "F2", Key::F2, "F3", Key::F3, "F4", Key::F4,
        "F5", Key::F5, "F6", Key::F6, "F7", Key::F7, "F8", Key::F8, "F9",
        Key::F9, "F10", Key::F10, "F11", Key::F11, "F12", Key::F12, "F13",
        Key::F13, "F14", Key::F14, "F15", Key::F15, "F16", Key::F16, "F17",
        Key::F17, "F18", Key::F18, "F19", Key::F19, "F20", Key::F20, "F21",
        Key::F21, "F22", Key::F22, "F23", Key::F23, "F24", Key::F24, "F25",
        Key::F25, "KP0", Key::KP0, "KP1", Key::KP1, "KP2", Key::KP2, "KP3",
        Key::KP3, "KP4", Key::KP4, "KP5", Key::KP5, "KP6", Key::KP6, "KP7",
        Key::KP7, "KP8", Key::KP8, "KP9", Key::KP9, "KPDecimal", Key::KPDecimal,
        "KPDivide", Key::KPDivide, "KPMultiply", Key::KPMultiply, "KPSubtract",
        Key::KPSubtract, "KPAdd", Key::KPAdd, "KPEnter", Key::KPEnter,
        "KPEqual", Key::KPEqual, "LeftShift", Key::LeftShift, "LeftControl",
        Key::LeftControl, "LeftAlt", Key::LeftAlt, "LeftSuper", Key::LeftSuper,
        "RightShift", Key::RightShift, "RightControl", Key::RightControl,
        "RightAlt", Key::RightAlt, "RightSuper", Key::RightSuper, "Menu",
        Key::Menu);

    m_state.new_enum("MouseButton", "D1", MouseButton::D1, "D2",
                     MouseButton::D2, "D3", MouseButton::D3, "D4",
                     MouseButton::D4, "D5", MouseButton::D5, "D6",
                     MouseButton::D6, "D7", MouseButton::D7, "D8",
                     MouseButton::D8, "Left", MouseButton::Left, "Right",
                     MouseButton::Right, "Middle", MouseButton::Middle);
}

void Script::set_globals() {
    m_state["RootEntity"] = ScriptUserTypes::ScriptEntity(m_entity);
    m_state["Entities"] = ScriptUserTypes::ScriptEntities(m_scene, &m_state);
    m_state["UI"] = ScriptUserTypes::ScriptUI(m_scene);
}

void Script::load_global_fields() {}

void Script::load_user_type_field(const std::string& name, sol::object object) {
    if (object.is<ScriptUserTypes::ScriptEntity>()) {
        m_exposed_fields.push_back(ScriptFieldEntity(
            name, object.as<ScriptUserTypes::ScriptEntity*>()));
    }
}

void Script::get_exposed_fields_state() {
    for (ScriptField& field : m_exposed_fields) {
        switch (field.get_type()) {
        case ScriptFieldType::Entity: {
            auto concrete_field = field.as<ScriptFieldEntity>();

            concrete_field->set_state(concrete_field->get_object()->get_id());
            break;
        }
        }
    }
}

void Script::set_exposed_fields_state() {
    for (ScriptField& field : m_exposed_fields) {
        switch (field.get_type()) {
        case ScriptFieldType::Entity: {
            auto concrete_field = field.as<ScriptFieldEntity>();

            concrete_field->set_state(concrete_field->get_object()->get_id());
            concrete_field->get_object()->set_entity(
                Entity(concrete_field->get_state(), m_scene));
            break;
        }
        }
    }
}

int count_occurrences(const std::string& str, const std::string& sub,
                      size_t start, size_t end) {
    int count = 0;
    size_t pos =
        str.find(sub, start); // Find the first occurrence of the substring

    while (pos != std::string::npos && pos < end) {
        count++;
        pos = str.find(sub, pos + 1); // Move to the next occurrence
    }

    return count;
}

void Script::parse_exposed_fields(const std::string& src) {
    sol::table globals = m_state.globals();

    std::vector<std::tuple<std::string, sol::object>> relevant_globals;
    for (auto pair : globals) {
        std::string name = pair.first.as<std::string>();

        if (name == "RootEntity" || name == "Entities" || name == "UI" ||
            name == "Entity") {
            continue;
        }

        std::string::size_type field_index = src.find(name);

        std::string::size_type tag_index = src.rfind("---@expose", field_index);
        if (tag_index == std::string::npos) {
            continue;
        }
        if (count_occurrences(src, "\n", tag_index, field_index) > 1) {
            continue;
        }

        relevant_globals.push_back(std::make_tuple<std::string, sol::object>(
            std::move(name), std::move(pair.second)));
    }

    for (auto& [name, object] : relevant_globals) {
        switch (object.get_type()) {
        case sol::type::number: {
            break;
        }
        case sol::type::boolean: {
            break;
        }
        case sol::type::string: {
            break;
        }
        case sol::type::userdata: {
            load_user_type_field(name, object);
            break;
        }
        default:
            break;
        }
    }
}
} // namespace Helios
