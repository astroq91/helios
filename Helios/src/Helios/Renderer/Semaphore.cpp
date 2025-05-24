#include "Semaphore.h"
#include "Helios/Core/Application.h"

namespace Helios {
Semaphore::~Semaphore() {
    if (m_initialized) {
        auto device =
            Application::get().get_vulkan_manager()->get_context().device;
        // Enqueue the destruction command
        Application::get().get_vulkan_manager()->enqueue_for_destruction(
            [device, semaphore = m_semaphore]() {
                vkDestroySemaphore(device, semaphore, nullptr);
            });
    }
}
void Semaphore::init() {
    m_initialized = true;
    auto& context = Application::get().get_vulkan_manager()->get_context();
    VkSemaphoreCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
    };
    if (vkCreateSemaphore(context.device, &info, nullptr, &m_semaphore) !=
        VK_SUCCESS) {
        HL_ERROR("Failed to create semaphore");
    }
}
} // namespace Helios
