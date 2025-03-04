#pragma once
#include <entt/entt.hpp>

#include "Helios/Scene/Scene.h"

#include "Helios/Scene/Entity.h"
#include "Project.h"

namespace Helios {
class AssetsBrowser {
  public:
    void on_update(Scene* scene, const Project& project);

};
} // namespace Helios
