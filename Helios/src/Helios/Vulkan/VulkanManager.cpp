#include "VulkanManager.h"

#include "Helios/Core/Application.h"

namespace Helios {
void VulkanManager::init() {
    auto& app = Application::get();

    m_context.Init();

    m_action_queues.resize(app.get_max_frames_in_flight());
    m_destruction_queues.resize(app.get_max_frames_in_flight());
}

void VulkanManager::on_update() {
    auto& app = Application::get();

    while (!m_action_queues[app.get_current_frame()].empty()) {
        auto& func = m_action_queues[app.get_current_frame()].front();
        func();
        m_action_queues[app.get_current_frame()].pop();
    }

    while (!m_destruction_queues[app.get_current_frame()].empty()) {
        auto& func = m_destruction_queues[app.get_current_frame()].front();
        func();
        m_destruction_queues[app.get_current_frame()].pop();
    }
}

VulkanManager::~VulkanManager() {
    // Destroy all the remaining resources
    for (auto& queue : m_destruction_queues) {
        size_t queue_size = queue.size();
        for (size_t i = 0; i < queue_size; i++) {
            auto& func = queue.front();
            func();
            queue.pop();
        }
    }
}

void VulkanManager::enqueue_deferred_action(
    uint32_t index, const std::function<void()>& callback) {
    m_action_queues[index].push(callback);
}

void VulkanManager::enqueue_for_destruction(
    const std::function<void()>& callback) {
    auto& app = Application::get();
    m_destruction_queues[app.get_current_frame()].push(callback);
}
} // namespace Helios
