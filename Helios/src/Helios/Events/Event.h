#pragma once
#include <functional>

#include "Helios/Core/Core.h"

namespace Helios {
enum class EventType {
    None = 0,
    WindowClose,
    WindowResize,
    WindowFocus,
    WindowLostFocus,
    WindowMoved,
    AppTick,
    AppUpdate,
    AppRender,
    KeyPressed,
    KeyReleased,
    KeyTyped,
    MouseButtonPressed,
    MouseButtonReleased,
    MouseMoved,
    MouseScrolled,
    FramebufferResize,
};

enum EventCategory {
    EventCategoryNone = 0,
    EventCategoryApplication = BIT(0),
    EventCategoryInput = BIT(1),
    EventCategoryKeyboard = BIT(2),
    EventCategoryMouse = BIT(3),
    EventCategoryMouseButton = BIT(4),
};

class Event {
  public:
    virtual EventType get_event_type() const = 0;
    virtual int get_category_flags() const = 0;
    virtual bool is_in_category(EventCategory category) const = 0;

    bool handled = false;
};

/* Helper class that defines necessary function.
 * To be inherited from by event classes */
template <EventType Type, int CategoryFlags> class EventBase : public Event {
  public:
    static constexpr EventType get_static_type() noexcept { return Type; }

    constexpr EventType get_event_type() const noexcept override {
        return Type;
    }

    constexpr int get_category_flags() const noexcept override {
        return CategoryFlags;
    }

    constexpr bool
    is_in_category(EventCategory category) const noexcept override {
        return (CategoryFlags & category) != EventCategoryNone;
    }
};

template <typename T>
concept DerivedFromEvent = std::is_base_of_v<Event, T>;

class EventDispatcher {
  public:
    explicit EventDispatcher(Event& e) : m_event(e) {}

    template <DerivedFromEvent T>
    bool dispatch(const std::function<bool(T&)>& func) {
        if (m_event.get_event_type() == T::get_static_type()) {
            m_event.handled = func(static_cast<T&>(m_event));
            return true;
        }
        return false;
    }

  private:
    Event& m_event;
};
} // namespace Helios
