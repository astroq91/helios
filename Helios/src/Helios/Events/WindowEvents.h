#pragma once
#include "Event.h"

namespace Helios {
class FramebufferResize
    : public EventBase<EventType::FramebufferResize, EventCategoryApplication> {
  public:
    FramebufferResize() = default;
};
} // namespace Helios
