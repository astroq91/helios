#include <functional>

#include "../Renderer/Renderer.h"
#include "Application.h"
#include "Helios/Events/WindowEvents.h"

#include <PxPhysicsAPI.h>
using namespace physx;
namespace Helios {
#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

Application* Application::s_instance = nullptr;

Application::Application(const ApplicationInfo& info)
    : m_max_frames_in_flight(info.max_frames_in_flight) {
    s_instance = this;

#ifdef _DEBUG
    m_asset_base_path = BASE_DIR;
#endif

    m_window = std::make_unique<Window>(WindowProps());
    m_window->set_event_callback_fn(BIND_EVENT_FN(on_event));
    m_window->set_title("Helios");

    m_vulkan_manager.init();
    m_asset_manager.init();
    m_renderer.init(m_max_frames_in_flight);
    m_physics_manager.init();

    m_imgui_layer = new ImGuiLayer();
    push_overlay(m_imgui_layer);
}

Application::~Application() {
    for (auto it = m_layer_stack.begin(); it != m_layer_stack.end(); ++it) {
        delete (*it);
    }
}

void Application::run() {
    m_current_timestep = std::chrono::system_clock::now();
    m_last_timestep = m_current_timestep;

    while (m_application_running) {
        m_current_timestep = std::chrono::system_clock::now();

        float timestep =
            std::chrono::duration<float>(m_current_timestep - m_last_timestep)
                .count();

        m_renderer.begin_frame();

        m_vulkan_manager.on_update();

        m_renderer.begin_recording();

        for (Layer* layer : m_layer_stack) {
            layer->on_update(timestep);
        }

        // ... ImGui stuff ...
        m_imgui_layer->begin();
        for (Layer* layer : m_layer_stack) {
            layer->on_imgui_render();
        }

        m_imgui_layer->end();

        m_window->on_update();

        m_renderer.end_recording();

        m_renderer.end_frame();

        m_current_frame = (m_current_frame + 1) % m_max_frames_in_flight;

        m_last_timestep = m_current_timestep;
    }

    m_renderer.shutdown();
}

void Application::push_layer(Layer* layer) {
    m_layer_stack.push_layer(layer);
    layer->on_attach();
}

void Application::push_overlay(Layer* layer) {
    m_layer_stack.push_overlay(layer);
    layer->on_attach();
}

void Application::on_event(Event& e) {
    EventDispatcher dispatcher(e);
    dispatcher.dispatch<FramebufferResize>([&](FramebufferResize& e) {
        m_renderer.set_framebuffer_resized(true);

        return false;
    });

    for (auto it = m_layer_stack.end(); it != m_layer_stack.begin();) {
        (*--it)->on_event(e);
        if (e.handled) {
            break;
        }
    }
}
} // namespace Helios
