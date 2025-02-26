#pragma once
#include <queue>

#include "Helios/Vulkan/VulkanContext.h"

namespace Helios {
class VulkanManager {
  public:
    void init();

    void on_update();

    VulkanManager() = default;
    ~VulkanManager();

    void enqueue_deferred_action(uint32_t index,
                                 const std::function<void()>& callback);

    /**
     * \brief Enqueue a destroy function. Provide the destruction and
     * deallocation calls inside the function.
     * \param callback The function.
     */
    void enqueue_for_destruction(const std::function<void()>& callback);

    const VulkanContext& get_context() const { return m_context; }

  private:
    VulkanContext m_context;

    std::vector<std::queue<std::function<void()>>> m_action_queues;

    // A vector of queues (one for each frame in flight). The queues contain
    // functions for destroying and deallocating vulkan objects.
    std::vector<std::queue<std::function<void()>>> m_destruction_queues;
};
} // namespace Helios
