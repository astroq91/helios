#pragma once
#include <entt/entt.hpp>

#include "Helios/Scene/Scene.h"

#include "Helios/Scene/Entity.h"
#include "Project.h"

namespace Helios {
class ComponentsBrowser {
  public:
    /**
     * \brief Call in ImGuiRender update loop to update the Components Browser
     * panel.
     * \param scene The scene.
     * \param selected_entity The currently selected entity.
     */
    void on_update(Scene* scene, Entity selected_entity,
                   const Project& project);

  private:
    char m_name_buffer[256]{0};

    bool m_show_add_component_modal = false;
    bool m_ShowChooseTextureModal = false;
};
} // namespace Helios
