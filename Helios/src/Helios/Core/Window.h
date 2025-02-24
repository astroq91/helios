#pragma once
#include "../Events/Event.h"
#include <GLFW/glfw3.h>
#include <functional>

namespace Helios {
using EventCallbackFn = std::function<void(Event&)>;

struct WindowProps {
    unsigned int width;
    unsigned int height;

    WindowProps(unsigned int width = 1920, unsigned int height = 1080)
        : width(width), height(height) {}
};

class Window {
  public:
    Window(const WindowProps& props = WindowProps());
    ~Window();

    void on_update();

    int get_width() const { return m_data.width; }
    int get_height() const { return m_data.height; }

    int getX() const {
        int xpos, ypos;
        glfwGetWindowPos(m_window, &xpos, &ypos);
        return xpos;
    }

    int getY() const {
        int xpos, ypos;
        glfwGetWindowPos(m_window, &xpos, &ypos);
        return ypos;
    }

    float get_aspect_ratio() const {
        return (float)m_data.width / (float)m_data.height;
    }

    void set_vsync(int value = 1);

    void set_event_callback_fn(const EventCallbackFn& fn) {
        m_data.fn = fn;

        // Need to set the user pointer here for some some fucking reason?
        // Data is scrambled if you just set the pointer when initializing glfw.
        glfwSetWindowUserPointer(m_window, &m_data);
    }

    void set_title(const std::string& title) {
        glfwSetWindowTitle(m_window, title.c_str());
    }

    GLFWwindow* get_native_window() const { return m_window; }

  private:
    GLFWwindow* m_window;

    struct WindowData {
        unsigned int width;
        unsigned int height;

        EventCallbackFn fn;
    };

    WindowData m_data;
};
} // namespace Helios
