#include "Window.h"

#include "../Events/KeyEvents.h"
#include "Application.h"
#include "GLFW/glfw3.h"
#include "Helios/Events/MouseEvents.h"
#include "Helios/Events/WindowEvents.h"

namespace Helios {
void error_callback(int error, const char* description);

Window::Window(const WindowProps& windowProps) {
    m_data.width = windowProps.width;
    m_data.height = windowProps.height;

    if (glfwPlatformSupported(GLFW_PLATFORM_X11)) {
        glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
    }

    if (!glfwInit()) {
        HL_CRITICAL("Failed to init GLFW!");
        return;
    }

    glfwSetErrorCallback(error_callback);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_window = glfwCreateWindow(m_data.width, m_data.height, "Window", nullptr,
                                nullptr);
    if (!m_window) {
        glfwTerminate();
        HL_CRITICAL(
            "Could not create window! Does your machine support OpenGL 4.6?");
        return;
    }

    glfwSetKeyCallback(m_window, [](GLFWwindow* window, int key, int scancode,
                                    int action, int mods) {
        WindowData* data = (WindowData*)glfwGetWindowUserPointer(window);
        switch (action) {
        case GLFW_PRESS: {
            KeyPressedEvent e((KeyCode)key, 0);
            data->fn(e);
            break;
        }
        case GLFW_RELEASE: {
            KeyReleasedEvent e((KeyCode)key);
            data->fn(e);
            break;
        }
        case GLFW_REPEAT: {
            KeyPressedEvent e((KeyCode)key, 1);
            data->fn(e);
            break;
        }
        }
    });

    glfwSetMouseButtonCallback(
        m_window, [](GLFWwindow* window, int button, int action, int mods) {
            WindowData* data = (WindowData*)glfwGetWindowUserPointer(window);
            switch (action) {
            case GLFW_PRESS: {
                MouseButtonPressedEvent e((MouseButton)button);
                data->fn(e);
                break;
            }
            case GLFW_RELEASE: {
                MouseButtonReleasedEvent e((MouseButton)button);
                data->fn(e);
                break;
            }
            }
        });

    glfwSetCursorPosCallback(
        m_window, [](GLFWwindow* window, double xpos, double ypos) {
            WindowData* data = (WindowData*)glfwGetWindowUserPointer(window);
            MouseMovedEvent e(xpos, ypos);
            data->fn(e);
        });

    glfwSetWindowSizeCallback(
        m_window, [](GLFWwindow* window, int width, int height) {
            WindowData* data = (WindowData*)glfwGetWindowUserPointer(window);
            data->width = width;
            data->height = height;
        });

    glfwSetFramebufferSizeCallback(
        m_window, [](GLFWwindow* window, int width, int height) {});

    glfwSetWindowCloseCallback(
        m_window, [](GLFWwindow* window) { Application::get().shutdown(); });

    glfwSetFramebufferSizeCallback(
        m_window, [](GLFWwindow* window, int width, int height) {
            WindowData* data = (WindowData*)glfwGetWindowUserPointer(window);
            FramebufferResize e;
            data->fn(e);
        });
}

Window::~Window() {
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void Window::on_update() {
    glfwPollEvents();
    // glfwSwapBuffers(m_window);
}

void Window::set_vsync(int value) { glfwSwapInterval(value); }

// Helper Functions //

void error_callback(int error, const char* description) {
    HL_ERROR("Error: {0}", description);
}
} // namespace Helios
