#pragma once
#include <cstdio>
#include <entt/entt.hpp>

#include "Scene.h"

namespace Helios {

class Entity {
  public:
    Entity()
        : m_handle(static_cast<entt::entity>(k_no_entity)), m_scene(nullptr) {}

    explicit Entity(const uint32_t handle, Scene* scene)
        : m_handle(static_cast<entt::entity>(handle)), m_scene(scene) {}

    operator entt::entity() const { return m_handle; }
    operator uint32_t() const { return static_cast<uint32_t>(m_handle); }

    entt::entity get_handle() const { return m_handle; }

    /**
     * \brief Adds a component of a specific type, with specified data. Or
     * returns the component if it already exists.
     * \tparam Type The type of the component.
     * \tparam Args
     * \param data Potential component data.
     */
    template <typename Type, typename... Args>
    Type& add_component(Args&&... data) {
        return m_scene->m_registry.get_or_emplace<Type>(
            m_handle, std::forward<Args>(data)...);
    }

    template <typename Type> void remove_component() {
        m_scene->m_registry.remove<Type>(m_handle);
    }

    /**
     * \brief Check if entity has component of specified type.
     * \tparam Type The component type.
     * \return Boolean, if the component exists.
     */
    template <typename Type> bool has_component() {
        return m_scene->m_registry.try_get<Type>(m_handle) != nullptr;
    }

    /**
     * \brief get a entity's component. Undefined if the entity does not have
     * the component. (TODO: Fix that)
     * \tparam Type The component type.
     * \return A reference to the component.
     */
    template <typename Type> Type& get_component() {
        return m_scene->m_registry.get<Type>(m_handle);
    }

    template <typename Type> Type* try_get_component() {
        return m_scene->m_registry.try_get<Type>(m_handle);
    }

    void update_rigid_body_mass(float value) {
        m_scene->update_rigid_body_mass(*this, value);
    }

    void update_rigid_body_static_friction(float value) {
        m_scene->update_rigid_body_static_friction(*this, value);
    }

    void update_rigid_body_dynamic_friction(float value) {
        m_scene->update_rigid_body_dynamic_friction(*this, value);
    }

    void update_rigid_body_restitution(float value) {
        m_scene->update_rigid_body_restitution(*this, value);
    }

  private:
    entt::entity m_handle;
    Scene* m_scene = nullptr;
};
} // namespace Helios
