#pragma once

#include <volk/volk.h>

#include "Helios/Core/Core.h"

namespace Helios {
struct Semaphore {
  public:
    static SharedPtr<Semaphore> create(bool vsync = true) {
        SharedPtr<Semaphore> sc = SharedPtr<Semaphore>::create();
        sc->init();
        return sc;
    }

    ~Semaphore();

    const VkSemaphore& get_vk_semaphore() const { return m_semaphore; }

  private:
    void init();

  private:
    VkSemaphore m_semaphore;
    bool m_initialized;
};
} // namespace Helios
