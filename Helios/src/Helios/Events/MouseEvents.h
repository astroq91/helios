#pragma once
#include "Event.h"
#include "KeyCodes.h"

namespace Helios {
class MouseButtonPressedEvent
    : public EventBase<EventType::MouseButtonPressed,
                       EventCategoryMouse | EventCategoryMouseButton> {
  public:
    MouseButtonPressedEvent(MouseButton button) : m_Button(button) {}

    MouseButton get_button() const { return m_Button; }

  private:
    MouseButton m_Button;
};

class MouseButtonReleasedEvent
    : public EventBase<EventType::MouseButtonReleased,
                       EventCategoryMouse | EventCategoryMouseButton> {
  public:
    MouseButtonReleasedEvent(MouseButton button) : m_button(button) {}

    MouseButton get_button() const { return m_button; }

  private:
    MouseButton m_button;
};

class MouseMovedEvent
    : public EventBase<EventType::MouseMoved, EventCategoryMouse> {
  public:
    MouseMovedEvent(int x, int y) : m_X(x), m_Y(y) {}

    std::tuple<double, double> get_pos() const { return {m_X, m_Y}; }

  private:
    double m_X, m_Y;
};
} // namespace Helios
