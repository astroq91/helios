#pragma once
#include "Event.h"
#include "KeyCodes.h"

namespace Helios {

class KeyPressedEvent
    : public EventBase<EventType::KeyPressed,
                       EventCategoryKeyboard | EventCategoryInput> {
  public:
    KeyPressedEvent(KeyCode key_code, int repeat_count)
        : m_key_code(key_code), m_repeat_count(repeat_count) {}

    KeyCode get_key_code() const { return m_key_code; }

    // Description:
    // Returns 1 if the key is being repeated, and 0 if not.
    int get_repeat_count() const { return m_repeat_count; }

  private:
    KeyCode m_key_code;
    int m_repeat_count;
};

class KeyReleasedEvent
    : public EventBase<EventType::KeyReleased,
                       EventCategoryKeyboard | EventCategoryInput> {
  public:
    KeyReleasedEvent(KeyCode key_code) : m_key_code(key_code) {}

    KeyCode get_key_code() const { return m_key_code; }

  private:
    KeyCode m_key_code;
};
} // namespace Helios
