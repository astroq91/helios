#pragma once
#include "../Events/Event.h"

namespace Helios {
class Layer {
  public:
    virtual ~Layer() = default;

    virtual void on_attach() {}

    virtual void on_detach() {}

    virtual void on_update(float ts) {}

    virtual void on_fixed_update() {}

    virtual void on_imgui_render() {}

    virtual void on_event(Event& e) {}
};
} // namespace Helios
