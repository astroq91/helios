#include "Script.h"
#include "Helios/Core/Application.h"
#include "Helios/Core/IOUtils.h"
#include "Helios/Core/Log.h"
#include "Helios/ECSComponents/Components.h"
#include "Helios/Events/Input.h"
#include "Helios/Scene/Entity.h"
#include "Helios/Scene/Scene.h"
#include "sol/variadic_args.hpp"
#include <fstream>
#include <glm/glm.hpp>
#include <sstream>

namespace Helios {

class Components {
  public:
    Components(Entity entity) : m_entity(entity) {}

    TransformComponent* get_transform() {
        return m_entity.try_get_component<TransformComponent>();
    }

    NameComponent* get_name() {
        return m_entity.try_get_component<NameComponent>();
    }

    CameraComponent* get_camera() {
        return m_entity.try_get_component<CameraComponent>();
    }

    DirectionalLightComponent* get_directional_light() {
        return m_entity.try_get_component<DirectionalLightComponent>();
    }

    PointLightComponent* get_point_light() {
        return m_entity.try_get_component<PointLightComponent>();
    }

    RigidBodyComponent* get_rigid_body() {
        return m_entity.try_get_component<RigidBodyComponent>();
    }

  private:
    Entity m_entity;
};

/**
 * Interface for a single entity.
 */
class ScriptEntity {
  public:
    ScriptEntity(Entity entity) : m_entity(entity), m_components(entity) {}
    Components* get_components() { return &m_components; }

  private:
    Entity m_entity;
    Components m_components;
};

/**
 * Used to manage all the entities in the scene.
 */
class ScriptEntities {
  public:
    ScriptEntities(Scene* scene, sol::state* state)
        : m_scene(scene), m_state(state) {}
    ScriptEntity create_entity(const std::string& name) {
        Entity entity = m_scene->create_entity(name);
        return ScriptEntity(entity);
    }

  private:
    Scene* m_scene;
    sol::state* m_state;
};

Script::Script(const std::string& src, ScriptLoadType load_type, Scene* scene,
               Entity entity)
    : m_scene(scene), m_entity(entity) {
    m_state.open_libraries(sol::lib::base, sol::lib::package);

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

    // Add the base dir to the package path to access the type annotations
    std::stringstream ss;
    ss << "package.path = package.path .. ';" << BASE_DIR << "?.lua'";

    m_state.script(ss.str());

    load_script(src, load_type);
}

void Script::load_script(const std::string& src, ScriptLoadType load_type) {
    if (load_type == ScriptLoadType::Source) {
        m_state.script(src);
    } else {

        std::ifstream file(IOUtils::resolve_path(
                               Application::get().get_asset_base_path(), src),
                           std::ios::ate | std::ios::binary);

        if (file.fail()) {
            HL_ERROR("[Scripting] Could not open script file: {0}", src);
        }

        size_t file_size = (size_t)file.tellg();
        std::vector<char> src(file_size);
        file.seekg(0);
        file.read(src.data(), file_size);
        file.close();

        std::string script(src.begin(), src.end());

        m_state.script(script);
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
}

void Script::expose_basic_types() {
    m_state.new_usertype<glm::vec3>("vec3", "x", &glm::vec3::x, "y",
                                    &glm::vec3::y, "z", &glm::vec3::z);
    m_state.new_usertype<glm::vec4>("vec4", "x", &glm::vec4::x, "y",
                                    &glm::vec4::y, "z", &glm::vec4::z, "w",
                                    &glm::vec4::w);
    m_state.new_usertype<glm::quat>("quat", "x", &glm::quat::x, "y",
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
    m_state.new_usertype<ScriptEntity>("Entity", "get_components",
                                       &ScriptEntity::get_components);

    m_state.new_usertype<ScriptEntities>("Entities", "create_entity",
                                         &ScriptEntities::create_entity);

    m_state.new_usertype<Components>(
        "Components", "get_transform", &Components::get_transform, "get_name",
        &Components::get_name, "get_camera", &Components::get_camera,
        "get_directional_light", &Components::get_directional_light,
        "get_point_light", &Components::get_point_light, "get_rigid_body",
        &Components::get_rigid_body);

    m_state.new_usertype<Input>("Input", "is_key_pressed",
                                &Input::is_key_pressed, "is_key_released",
                                &Input::is_key_released);
}

void Script::expose_component_user_types() {

    m_state.new_usertype<TransformComponent>(
        "Transform", "position", &TransformComponent::position, "rotation",
        &TransformComponent::rotation, "scale", &TransformComponent::scale);

    m_state.new_usertype<NameComponent>("Name", "name", &NameComponent::name);

    m_state.new_usertype<CameraComponent>(
        "Camera", "fov_y", &CameraComponent::fovY, "near",
        &CameraComponent::near, "far", &CameraComponent::far);

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

    // TODO: Maybe Mesh?
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
    // Only have a root entity if the script is bound to an entity
    if (m_entity != k_no_entity) {
        m_state["RootEntity"] = ScriptEntity(m_entity);
    }
    m_state["Entities"] = ScriptEntities(m_scene, &m_state);
}

} // namespace Helios
