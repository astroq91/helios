#pragma once
#include <chrono>
#include <memory>

#include "Helios/Assets/AssetManager.h"
#include "Helios/ImGui/ImGuiLayer.h"
#include "Helios/Physics/PhysicsManager.h"
#include "Helios/Renderer/Renderer.h"
#include "Helios/Vulkan/VulkanManager.h"
#include "LayerStack.h"
#include "Window.h"

namespace Helios {
struct ApplicationInfo {
    uint32_t max_frames_in_flight = 2;
};

class Application {
  public:
    Application(const ApplicationInfo& info);
    ~Application();
    void run();

    static Application& get() { return *s_instance; }

    uint32_t get_max_frames_in_flight() const { return m_max_frames_in_flight; }
    uint32_t get_current_frame() const { return m_current_frame; }

    VulkanManager* get_vulkan_manager() { return &m_vulkan_manager; }
    AssetManager& get_asset_manager() { return m_asset_manager; }
    Physics::PhysicsManager& get_physics_manager() { return m_physics_manager; }

    Renderer& get_renderer() { return m_renderer; }

    void shutdown() { m_application_running = false; }

    void push_layer(Layer* layer);
    void push_overlay(Layer* layer);

    void on_event(Event& e);

    Window& get_window() const { return *m_window; }
    GLFWwindow* get_native_window() const {
        return m_window->get_native_window();
    }
    ImGuiLayer* get_imgui_layer() const { return m_imgui_layer; }

    const std::string& get_asset_base_path() const { return m_asset_base_path; }
    void set_asset_base_path(const std::string& base_path) {
        m_asset_base_path = base_path;
    }

  private:
    VulkanManager m_vulkan_manager; // Destroy the vulkan context last
    AssetManager m_asset_manager;
    Physics::PhysicsManager m_physics_manager;

    Renderer m_renderer;

    LayerStack m_layer_stack;
    ImGuiLayer* m_imgui_layer;

    std::unique_ptr<Window> m_window;

    static Application* s_instance;
    bool m_application_running = true;

    uint32_t m_current_frame = 0;
    uint32_t m_max_frames_in_flight;

    std::chrono::time_point<std::chrono::system_clock> m_last_timestep;
    std::chrono::time_point<std::chrono::system_clock> m_current_timestep;

    std::string m_asset_base_path;
};

Application* create_application();
} // namespace Helios
